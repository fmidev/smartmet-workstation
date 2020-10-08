#!/usr/bin/perl

my $file = shift;

local $/;
local *F;
open(F, "<$file\0") || die "file: $!\n";
$text = <F>;
close(F);

$text =~ s/\n//gs;
$text =~ s/\r//gs;


while ($text =~ /<shape\s+id="([^"]+)"[^>]*nimi="([^"]+)"[^>]*>(.*?)<\/shape>/gis) {

my $id = $1;
my $name = $2;


open(TMP,"+>$name.svg");
print TMP "#$name\n\"$3\"\n";
close(TMP);

}
