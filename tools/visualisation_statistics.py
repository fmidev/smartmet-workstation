"""Generate an HTML report and machine-readable JSON from SmartMet trace logs.

Usage: python tools/visualisation_statistics.py [log_directory] [output_directory]
Only Python's standard library is required.
"""
import argparse
from collections import defaultdict
from decimal import Decimal
from html import escape
import json
from pathlib import Path
import re


EVENT = re.compile(
    rb"^\[(?P<time>[^\]]+)\]'trace' Vis: \[(?P<thread>[^\]]+)\] Operation lasted "
    rb"(?P<seconds>\d+(?:\.\d+)?)s for NFmi(?P<view>Station|IsoLine)View::"
    rb"(?P<operation>Draw|CalcMacroParamMatrix: macroParam calculations) by: "
    rb"'(?P<name>.*)' \[(?P<context>map-view=[^\]]+)\]\s*$"
)
TARGET = re.compile(rb"Operation lasted .* for NFmi(?:Station|IsoLine)View::(?:Draw\b|CalcMacroParamMatrix\b)")


def decode(value):
    try:
        return value.decode('utf-8')
    except UnicodeDecodeError:
        return value.decode('cp1252')


class Stats:
    def __init__(self):
        self.count = 0
        self.total = Decimal(0)
        self.minimum = None
        self.maximum = None

    def add(self, value):
        self.count += 1
        self.total += value
        self.minimum = value if self.minimum is None else min(value, self.minimum)
        self.maximum = value if self.maximum is None else max(value, self.maximum)

    def merge(self, other):
        if other.count:
            self.count += other.count
            self.total += other.total
            self.minimum = other.minimum if self.minimum is None else min(self.minimum, other.minimum)
            self.maximum = other.maximum if self.maximum is None else max(self.maximum, other.maximum)

    def export(self):
        return dict(count=self.count,
                    average_s=float(self.total / self.count) if self.count else None,
                    minimum_s=float(self.minimum) if self.count else None,
                    maximum_s=float(self.maximum) if self.count else None)


def pair():
    return [Stats(), Stats()]


def parse_file(path):
    groups = defaultdict(pair)
    pending = defaultdict(list)
    context_kinds = defaultdict(set)
    macro_names = set()
    diagnostics = dict(file=path.name, matched_draws=0, matched_calculations=0,
                       unparsed_target_lines=0, unparsed_examples=[], first_event=None, last_event=None)
    with path.open('rb') as source:
        for number, line in enumerate(source, 1):
            if b'Operation lasted ' not in line:
                continue
            match = EVENT.match(line)
            if not match:
                if TARGET.search(line):
                    diagnostics['unparsed_target_lines'] += 1
                    if len(diagnostics['unparsed_examples']) < 5:
                        diagnostics['unparsed_examples'].append(number)
                continue
            name = decode(match['name'])
            seconds = Decimal(match['seconds'].decode('ascii'))
            timestamp = decode(match['time'])
            diagnostics['first_event'] = diagnostics['first_event'] or timestamp
            diagnostics['last_event'] = timestamp
            # Calculation records can use a base class name. Associate them with
            # the following Draw using thread, name, and full layer coordinates.
            context = (match['thread'], match['name'], match['context'])
            if match['operation'] != b'Draw':
                macro_names.add(name)
                pending[context].append(seconds)
                diagnostics['matched_calculations'] += 1
            else:
                kind = 'Symbol' if match['view'] == b'Station' else 'Isoline'
                context_kinds[context].add(kind)
                groups[name, kind][0].add(seconds)
                for value in pending.pop(context, []):
                    groups[name, kind][1].add(value)
                diagnostics['matched_draws'] += 1
    diagnostics['calculations_without_following_draw'] = sum(len(values) for values in pending.values())
    # A file may end during a draw. Retain its calculation when the same
    # thread/name/coordinates has one unambiguous observed visualisation type.
    diagnostics['unpaired_calculations'] = 0
    for context, values in pending.items():
        if len(context_kinds[context]) == 1:
            kind = next(iter(context_kinds[context]))
            for value in values:
                groups[decode(context[1]), kind][1].add(value)
        else:
            diagnostics['unpaired_calculations'] += len(values)
    return groups, macro_names, diagnostics


def rows(groups, macro_names):
    result = []
    for (name, kind), (draw, calc) in groups.items():
        category = 'MacroParam' if name in macro_names or not re.search(r' \([^()]+\)$', name) else 'Normal parameter'
        result.append(dict(name=name, visualisation=kind, category=category,
                           draw=draw.export(), calculation=calc.export()))
    return sorted(result, key=lambda row: (row['category'] == 'MacroParam',
                                          -(row['draw']['average_s'] or 0),
                                          row['name'].casefold(), row['name'], row['visualisation']))


def table(data):
    parts = ['<div class="scroll"><table><thead><tr><th>Name</th><th>Visualisation</th><th>Draw count</th>'
             '<th>Draw avg (s)</th><th>Draw min (s)</th><th>Draw max (s)</th>'
             '<th>Calc count</th><th>Calc avg (s)</th><th>Calc min (s)</th><th>Calc max (s)</th></tr></thead><tbody>']
    for row in data:
        cells = [escape(row['name']), row['visualisation']]
        for metric in ('draw', 'calculation'):
            stats = row[metric]
            cells.append(f"{stats['count']:,}" if metric == 'draw' or row['category'] == 'MacroParam' else '—')
            cells.extend('—' if stats[key] is None else f'{stats[key]:.6f}' for key in ('average_s', 'minimum_s', 'maximum_s'))
        parts.append('<tr>' + ''.join(f'<td>{cell}</td>' for cell in cells) + '</tr>')
    return ''.join(parts) + '</tbody></table></div>'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log_directory', nargs='?', type=Path, default=Path('log'))
    parser.add_argument('output_directory', nargs='?', type=Path, default=Path('log/statistics'))
    args = parser.parse_args()
    files = sorted(args.log_directory.glob('*.txt'))
    if not files:
        parser.error('No .txt log files found')
    all_groups = defaultdict(pair)
    macro_names = set()
    parsed = []
    for path in files:
        groups, names, diagnostics = parse_file(path)
        macro_names.update(names)
        parsed.append((groups, diagnostics))
        for key, stats in groups.items():
            for index in (0, 1):
                all_groups[key][index].merge(stats[index])
        print(json.dumps(diagnostics), flush=True)
    sections = [dict(title='All files combined', rows=rows(all_groups, macro_names))]
    sections.extend(dict(title=diagnostics['file'], diagnostics=diagnostics, rows=rows(groups, macro_names))
                    for groups, diagnostics in parsed)
    notes = ('All times are seconds. Rows group exact layer name and visualisation type across all map views, rows, columns and layers. '
             'Each category is sorted by average draw time, highest first; ties are sorted by name. '
             'Draw count counts Draw records; averages are weighted by individual records, including logged zero durations. '
             'Draw duration already includes calculation time: do not add the two. '
             'Calculation statistics use recorded calculations associated with the following draw on the same thread, name and layer coordinates within each file. '
             'For calculations without a following draw at file end, the type is taken from earlier draws at those same coordinates if unambiguous. '
             'Cached draws may have no calculation record, so calculation count can differ from draw count. '
             'A name observed in a calculation record is a MacroParam; otherwise names ending in “ (source)” are normal parameters and remaining names are MacroParams. '
             'Missing timing values are shown as —. Six decimal places preserve small averages; input measurements retain their original precision. '
             'Each input file has its own section, including the two files dated September 28. Only logged operations are represented; partial days are not extrapolated.')
    html = ['<!doctype html><html lang="en"><meta charset="utf-8"><title>SmartMet visualisation statistics</title>',
            '<style>body{font:15px system-ui;margin:32px;color:#182536}h1,h2,h3{color:#164875}p{max-width:1100px;line-height:1.6}'
            'table{border-collapse:collapse;width:100%;font-variant-numeric:tabular-nums}th,td{padding:8px 12px;border-bottom:1px solid #dde3ea;text-align:right;white-space:nowrap}'
            'th:first-child,td:first-child,th:nth-child(2),td:nth-child(2){text-align:left}th{background:#eaf1f8}tr:nth-child(even){background:#f7f9fb}'
            '.scroll{overflow:auto}nav a{display:block;margin:6px 0}section{margin-top:42px}</style>',
            '<h1>SmartMet visualisation statistics</h1>', f'<p>{escape(notes)}</p>', '<nav>']
    for index, section in enumerate(sections):
        html.append(f'<a href="#section{index}">{escape(section["title"])}</a>')
    html.append('</nav>')
    for index, section in enumerate(sections):
        html.append(f'<section id="section{index}"><h2>{escape(section["title"])}</h2>')
        if 'diagnostics' in section:
            d = section['diagnostics']
            html.append(f'<p>Event coverage: {escape(str(d["first_event"]))} – {escape(str(d["last_event"]))}<br>'
                        f'Draws: {d["matched_draws"]:,}; calculations: {d["matched_calculations"]:,}; '
                        f'calculations without a following draw: {d["calculations_without_following_draw"]:,}; '
                        f'unpaired calculations: {d["unpaired_calculations"]:,}; unparsed target lines: {d["unparsed_target_lines"]:,}.</p>')
        for category in ('Normal parameter', 'MacroParam'):
            html.append(f'<h3>{category}</h3>' + table([row for row in section['rows'] if row['category'] == category]))
        html.append('</section>')
    html.append('</html>')
    args.output_directory.mkdir(parents=True, exist_ok=True)
    (args.output_directory / 'visualisation_statistics.html').write_text(''.join(html), encoding='utf-8')
    (args.output_directory / 'visualisation_statistics.json').write_text(json.dumps(dict(notes=notes, sections=sections), ensure_ascii=False, indent=2), encoding='utf-8')
    # Every parsed operation must be accounted for in the report or diagnostics.
    assert sum(row['draw']['count'] for row in sections[0]['rows']) == sum(d['matched_draws'] for _, d in parsed)
    assert sum(row['calculation']['count'] for row in sections[0]['rows']) + sum(d['unpaired_calculations'] for _, d in parsed) == sum(d['matched_calculations'] for _, d in parsed)
    print(f'Report written to {args.output_directory.resolve()}')


if __name__ == '__main__':
    main()
