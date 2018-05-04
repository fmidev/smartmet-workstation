// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::WeatherStory::overview
 */
// ======================================================================

#include "WeatherStory.h"
#include "CloudinessStory.h"
#include "CloudinessStoryTools.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "PeriodPhraseFactory.h"
#include "PrecipitationPeriodTools.h"
#include "PrecipitationStoryTools.h"
#include <calculator/RangeAcceptor.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include <calculator/WeatherResult.h>
#include "WeekdayTools.h"

#include <boost/lexical_cast.hpp>
#include <vector>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief A structure for storing rain dispatch information
 *
 * The phrases are time phrases to be passed onto the story generating
 * subroutines. Either one or both may be empty strings.
 */
// ----------------------------------------------------------------------

struct rain_dispatch_data
{
  int index;
  const char* phrase1;
  const char* phrase2;
};

// ----------------------------------------------------------------------
/*!
 * \brief Second table from page \ref page_rain_oneday
 *
 * All 36 unique one day rains mapped onto 4 different reduced
 * cases with two time phrase parameters
 */
// ----------------------------------------------------------------------

rain_dispatch_data one_day_cases[37] = {{0, "", ""},
                                        {2, "aamulla", "aamupaivasta alkaen"},
                                        {2, "aamulla", "keskipaivasta alkaen"},
                                        {3, "aamupaivalla", "iltapaivasta alkaen"},
                                        {3, "aamupaivasta alkaen", "illalla"},
                                        {1, "aamupaivasta alkaen", ""},
                                        {3, "aamusta alkaen", "illalla"},
                                        {3, "aamusta alkaen", "illasta alkaen"},
                                        {2, "aamusta alkaen", "iltapaivasta alkaen"},
                                        {3, "aamusta alkaen", "iltapaivasta alkaen"},
                                        {1, "aamusta alkaen", ""},
                                        {2, "aamuyolla", "paivalla"},
                                        {4, "aamulla", ""},
                                        {4, "aamupaivalla", ""},
                                        {4, "illalla", ""},
                                        {4, "illasta alkaen", ""},
                                        {4, "iltapaivalla", ""},
                                        {4, "iltapaivasta alkaen", ""},
                                        {4, "iltayolla", ""},
                                        {4, "keskipaivalla", ""},
                                        {4, "paivalla", ""},
                                        {1, "iltapaivalla", ""},
                                        {1, "iltapaivasta alkaen", ""},
                                        {3, "keskipaivasta alkaen", "illalla"},
                                        {1, "keskipaivasta alkaen", ""},
                                        {2, "keskiyolla", "paivalla"},
                                        {2, "", "aamupaivasta alkaen"},
                                        {3, "", "illalla"},
                                        {3, "", "iltapaivasta alkaen"},
                                        {2, "", "keskipaivasta alkaen"},
                                        {1, "", ""},
                                        {3, "paivalla", "iltapaivasta alkaen"},
                                        {1, "paivalla", ""},
                                        {2, "yolla", "aamupaivasta alkaen"},
                                        {2, "yolla", "aamusta alkaen"},
                                        {2, "yolla", "keskipaivasta alkaen"},
                                        {2, "yolla", "paivalla"}};

// ----------------------------------------------------------------------
/*!
 * \brief Table from page \ref page_rain_oneday
 *
 * All possible one day rains mapped onto 36 unique cases
 */
// ----------------------------------------------------------------------

int one_day_forecasts[301] = {
    0,   // empty place holder
    25,  // 1. Keskiyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    25,  // 2. Keskiyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    25,  // 3. Keskiyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 4. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 5. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 6. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    12,  // 7. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 8. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 9. [Enimmakseen selkeaa], aamulla [sadetta]
    13,  // 10. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    13,  // 11. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    19,  // 12. [Enimmakseen selkeaa], keskipaivalla [sadetta]
    19,  // 13. [Enimmakseen selkeaa], keskipaivalla [sadetta]
    16,  // 14. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 15. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 16. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 17. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 18. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    14,  // 19. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 20. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 21. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 22. [Enimmakseen selkeaa], illalla [sadetta]
    18,  // 23. [Enimmakseen selkeaa], iltayolla [sadetta]
    18,  // 24. [Enimmakseen selkeaa], iltayolla [sadetta]
    25,  // 25. Keskiyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    25,  // 26. Keskiyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 27. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 28. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 29. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    12,  // 30. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 31. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 32. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 33. [Enimmakseen selkeaa], aamulla [sadetta]
    13,  // 34. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    13,  // 35. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    19,  // 36. [Enimmakseen selkeaa], keskipaivalla [sadetta]
    16,  // 37. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 38. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 39. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 40. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 41. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    14,  // 42. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 43. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 44. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 45. [Enimmakseen selkeaa], illalla [sadetta]
    18,  // 46. [Enimmakseen selkeaa], iltayolla [sadetta]
    18,  // 47. [Enimmakseen selkeaa], iltayolla [sadetta]
    25,  // 48. Keskiyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 49. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 50. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 51. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    12,  // 52. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 53. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 54. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 55. [Enimmakseen selkeaa], aamulla [sadetta]
    13,  // 56. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    13,  // 57. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    13,  // 58. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    19,  // 59. [Enimmakseen selkeaa], keskipaivalla [sadetta]
    16,  // 60. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 61. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 62. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 63. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 64. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    14,  // 65. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 66. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 67. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 68. [Enimmakseen selkeaa], illalla [sadetta]
    18,  // 69. [Enimmakseen selkeaa], iltayolla [sadetta]
    25,  // 70. Keskiyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 71. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 72. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 73. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    12,  // 74. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 75. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 76. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 77. [Enimmakseen selkeaa], aamulla [sadetta]
    13,  // 78. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    13,  // 79. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    20,  // 80. [Enimmakseen selkeaa], paivalla [sadetta]
    20,  // 81. [Enimmakseen selkeaa], paivalla [sadetta]
    16,  // 82. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 83. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 84. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 85. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    14,  // 86. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 87. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 88. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 89. [Enimmakseen selkeaa], illalla [sadetta]
    15,  // 90. [Enimmakseen selkeaa], illasta alkaen [sadetta]
    36,  // 91. Yolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    36,  // 92. Yolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    11,  // 93. Aamuyolla [sadetta], paivalla [enimmakseen selkeaa] ja poutaa
    12,  // 94. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 95. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 96. [Enimmakseen selkeaa], aamulla [sadetta]
    12,  // 97. [Enimmakseen selkeaa], aamulla [sadetta]
    13,  // 98. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    13,  // 99. [Enimmakseen selkeaa], aamupaivalla [sadetta]
    20,  // 100. [Enimmakseen selkeaa], paivalla [sadetta]
    20,  // 101. [Enimmakseen selkeaa], paivalla [sadetta]
    20,  // 102. [Enimmakseen selkeaa], paivalla [sadetta]
    16,  // 103. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 104. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    16,  // 105. [Enimmakseen selkeaa], iltapaivalla [sadetta]
    17,  // 106. [Enimmakseen selkeaa], iltapaivasta alkaen [sadetta]
    17,  // 107. [Enimmakseen selkeaa], iltapaivasta alkaen [sadetta]
    14,  // 108. [Enimmakseen selkeaa], illalla [sadetta]
    15,  // 109. [Enimmakseen selkeaa], illasta alkaen [sadetta]
    15,  // 110. [Enimmakseen selkeaa], illasta alkaen [sadetta]
    34,  // 111. Yolla [sadetta], aamusta alkaen [enimmakseen selkeaa] ja poutaa
    34,  // 112. Yolla [sadetta], aamusta alkaen [enimmakseen selkeaa] ja poutaa
    34,  // 113. Yolla [sadetta], aamusta alkaen [enimmakseen selkeaa] ja poutaa
    1,   // 114. Aamulla [sadetta], aamupaivasta alkaen [enimmakseen selkeaa] ja poutaa
    1,   // 115. Aamulla [sadetta], aamupaivasta alkaen [enimmakseen selkeaa] ja poutaa
    2,   // 116. Aamulla [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    2,   // 117. Aamulla [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    3,   // 118. Aamupaivalla [sadetta], iltapaivasta alkaen poutaa
    31,  // 119. Paivalla [sadetta], iltapaivasta alkaen poutaa
    31,  // 120. Paivalla [sadetta], iltapaivasta alkaen poutaa
    32,  // 121. Paivalla [sadetta]
    32,  // 122. Paivalla [sadetta]
    21,  // 123. Iltapaivalla [sadetta]
    21,  // 124. Iltapaivalla [sadetta]
    22,  // 125. Iltapaivasta alkaen [sadetta]
    22,  // 126. Iltapaivasta alkaen [sadetta]
    22,  // 127. Iltapaivasta alkaen [sadetta]
    15,  // 128. [Enimmakseen selkeaa], illasta alkaen [sadetta]
    15,  // 129. [Enimmakseen selkeaa], illasta alkaen [sadetta]
    34,  // 130. Yolla [sadetta], aamusta alkaen [enimmakseen selkeaa] ja poutaa
    34,  // 131. Yolla [sadetta], aamusta alkaen [enimmakseen selkeaa] ja poutaa
    34,  // 132. Yolla [sadetta], aamusta alkaen [enimmakseen selkeaa] ja poutaa
    26,  // 133. [sadetta], aamupaivasta alkaen [enimmakseen selkeaa] ja poutaa
    2,   // 134. Aamulla [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    2,   // 135. Aamulla [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    8,   // 136. Aamusta alkaen [sadetta], iltapaivasta alkaen [enimmakseen selkeaa] ja poutaa
    8,   // 137. Aamusta alkaen [sadetta], iltapaivasta alkaen [enimmakseen selkeaa] ja poutaa
    32,  // 138. Paivalla [sadetta]
    32,  // 139. Paivalla [sadetta]
    32,  // 140. Paivalla [sadetta]
    23,  // 141. Keskipaivasta alkaen [sadetta], illalla poutaa
    23,  // 142. Keskipaivasta alkaen [sadetta], illalla poutaa
    22,  // 143. Iltapaivasta alkaen [sadetta]
    22,  // 144. Iltapaivasta alkaen [sadetta]
    22,  // 145. Iltapaivasta alkaen [sadetta]
    14,  // 146. [Enimmakseen selkeaa], illalla [sadetta]
    14,  // 147. [Enimmakseen selkeaa], illalla [sadetta]
    34,  // 148. Yolla [sadetta], aamusta alkaen [enimmakseen selkeaa] ja poutaa
    33,  // 149. Yolla [sadetta], aamupaivasta alkaen [enimmakseen selkeaa] ja poutaa
    33,  // 150. Yolla [sadetta], aamupaivasta alkaen [enimmakseen selkeaa] ja poutaa
    35,  // 151. Yolla [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    29,  // 152. [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    9,   // 153. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    9,   // 154. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    9,   // 155. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    9,   // 156. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    4,   // 157. Aamupaivasta alkaen [sadetta], illalla poutaa
    4,   // 158. Aamupaivasta alkaen [sadetta], illalla poutaa
    23,  // 159. Keskipaivasta alkaen [sadetta], illalla poutaa
    24,  // 160. Keskipaivasta alkaen [sadetta]
    22,  // 161. Iltapaivasta alkaen [sadetta]
    22,  // 162. Iltapaivasta alkaen [sadetta]
    22,  // 163. Iltapaivasta alkaen [sadetta]
    22,  // 164. Iltapaivasta alkaen [sadetta]
    26,  // 165. [sadetta], aamupaivasta alkaen [enimmakseen selkeaa] ja poutaa
    26,  // 166. [sadetta], aamupaivasta alkaen [enimmakseen selkeaa] ja poutaa
    29,  // 167. [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    29,  // 168. [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    28,  // 169. [sadetta], iltapaivasta alkaen poutaa
    9,   // 170. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    9,   // 171. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    9,   // 172. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    7,   // 173. Aamusta alkaen [sadetta], illasta alkaen poutaa
    4,   // 174. Aamupaivasta alkaen [sadetta], illalla poutaa
    4,   // 175. Aamupaivasta alkaen [sadetta], illalla poutaa
    24,  // 176. Keskipaivasta alkaen [sadetta]
    24,  // 177. Keskipaivasta alkaen [sadetta]
    22,  // 178. Iltapaivasta alkaen [sadetta]
    22,  // 179. Iltapaivasta alkaen [sadetta]
    22,  // 180. Iltapaivasta alkaen [sadetta]
    26,  // 181. [sadetta], aamupaivasta alkaen [enimmakseen selkeaa] ja poutaa
    29,  // 182. [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    29,  // 183. [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    28,  // 184. [sadetta], iltapaivasta alkaen poutaa
    28,  // 185. [sadetta], iltapaivasta alkaen poutaa
    9,   // 186. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    6,   // 187. Aamusta alkaen [sadetta], illalla poutaa
    6,   // 188. Aamusta alkaen [sadetta], illalla poutaa
    6,   // 189. Aamusta alkaen [sadetta], illalla poutaa
    4,   // 190. Aamupaivasta alkaen [sadetta], illalla poutaa
    5,   // 191. Aamupaivasta alkaen [sadetta]
    24,  // 192. Keskipaivasta alkaen [sadetta]
    24,  // 193. Keskipaivasta alkaen [sadetta]
    22,  // 194. Iltapaivasta alkaen [sadetta]
    22,  // 195. Iltapaivasta alkaen [sadetta]
    29,  // 196. [sadetta], keskipaivasta alkaen [enimmakseen selkeaa],  poutaa
    29,  // 197. [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    28,  // 198. [sadetta], iltapaivasta alkaen poutaa
    28,  // 199. [sadetta], iltapaivasta alkaen poutaa
    28,  // 200. [sadetta], iltapaivasta alkaen poutaa
    9,   // 201. Aamusta alkaen [sadetta], iltapaivasta alkaen poutaa
    6,   // 202. Aamusta alkaen [sadetta], illalla poutaa
    6,   // 203. Aamusta alkaen [sadetta], illalla poutaa
    6,   // 204. Aamusta alkaen [sadetta], illalla poutaa
    4,   // 205. Aamupaivasta alkaen [sadetta], illalla poutaa
    5,   // 206. Aamupaivasta alkaen [sadetta]
    24,  // 207. Keskipaivasta alkaen [sadetta]
    24,  // 208. Keskipaivasta alkaen [sadetta]
    22,  // 209. Iltapaivasta alkaen [sadetta]
    29,  // 210. [sadetta], keskipaivasta alkaen [enimmakseen selkeaa] ja poutaa
    28,  // 211. [sadetta], iltapaivasta alkaen poutaa
    28,  // 212. [sadetta], iltapaivasta alkaen poutaa
    28,  // 213. [sadetta], iltapaivasta alkaen poutaa
    28,  // 214. [sadetta], iltapaivasta alkaen poutaa
    6,   // 215. Aamusta alkaen [sadetta], illalla poutaa
    6,   // 216. Aamusta alkaen [sadetta], illalla poutaa
    6,   // 217. Aamusta alkaen [sadetta], illalla poutaa
    10,  // 218. Aamusta alkaen [sadetta]
    5,   // 219. Aamupaivasta alkaen [sadetta]
    5,   // 220. Aamupaivasta alkaen [sadetta]
    24,  // 221. Keskipaivasta alkaen [sadetta]
    24,  // 222. Keskipaivasta alkaen [sadetta]
    28,  // 223. [sadetta], iltapaivasta alkaen poutaa
    28,  // 224. [sadetta], iltapaivasta alkaen poutaa
    28,  // 225. [sadetta], iltapaivasta alkaen poutaa
    28,  // 226. [sadetta], iltapaivasta alkaen poutaa
    27,  // 227. [sadetta], illalla poutaa
    6,   // 228. Aamusta alkaen [sadetta], illalla poutaa
    6,   // 229. Aamusta alkaen [sadetta], illalla poutaa
    10,  // 230. Aamusta alkaen [sadetta]
    10,  // 231. Aamusta alkaen [sadetta]
    5,   // 232. Aamupaivasta alkaen [sadetta]
    5,   // 233. Aamupaivasta alkaen [sadetta]
    24,  // 234. Keskipaivasta alkaen [sadetta]
    28,  // 235. [sadetta], iltapaivasta alkaen poutaa
    28,  // 236. [sadetta], iltapaivasta alkaen poutaa
    28,  // 237. [sadetta], iltapaivasta alkaen poutaa
    27,  // 238. [sadetta], illalla poutaa
    27,  // 239. [sadetta], illalla poutaa
    6,   // 240. Aamusta alkaen [sadetta], illalla poutaa
    10,  // 241. Aamusta alkaen [sadetta]
    10,  // 242. Aamusta alkaen [sadetta]
    10,  // 243. Aamusta alkaen [sadetta]
    10,  // 244. Aamusta alkaen [sadetta]
    30,  // 245. [sadetta]
    28,  // 246. [sadetta], iltapaivasta alkaen poutaa
    28,  // 247. [sadetta], iltapaivasta alkaen poutaa
    27,  // 248. [sadetta], illalla poutaa
    27,  // 249. [sadetta], illalla poutaa
    27,  // 250. [sadetta], illalla poutaa
    10,  // 251. Aamusta alkaen [sadetta]
    10,  // 252. Aamusta alkaen [sadetta]
    10,  // 253. Aamusta alkaen [sadetta]
    10,  // 254. Aamusta alkaen [sadetta]
    10,  // 255. Aamusta alkaen [sadetta]
    28,  // 256. [sadetta], iltapaivasta alkaen poutaa
    27,  // 257. [sadetta], illalla poutaa
    27,  // 258. [sadetta], illalla poutaa
    27,  // 259. [sadetta], illalla poutaa
    30,  // 260. [sadetta]
    10,  // 261. Aamusta alkaen [sadetta]
    10,  // 262. Aamusta alkaen [sadetta]
    10,  // 263. Aamusta alkaen [sadetta]
    10,  // 264. Aamusta alkaen [sadetta]
    27,  // 265. [sadetta], illalla poutaa
    27,  // 266. [sadetta], illalla poutaa
    27,  // 267. [sadetta], illalla poutaa
    30,  // 268. [sadetta]
    30,  // 269. [sadetta]
    10,  // 270. Aamusta alkaen [sadetta]
    10,  // 271. Aamusta alkaen [sadetta]
    10,  // 272. Aamusta alkaen [sadetta]
    27,  // 273. [sadetta], illalla poutaa
    27,  // 274. [sadetta], illalla poutaa
    30,  // 275. [sadetta]
    30,  // 276. [sadetta]
    30,  // 277. [sadetta]
    10,  // 278. Aamusta alkaen [sadetta]
    10,  // 279. Aamusta alkaen [sadetta]
    27,  // 280. [sadetta], illalla poutaa
    30,  // 281. [sadetta]
    30,  // 282. [sadetta]
    30,  // 283. [sadetta]
    30,  // 284. [sadetta]
    10,  // 285. Aamusta alkaen [sadetta]
    30,  // 286. [sadetta]
    30,  // 287. [sadetta]
    30,  // 288. [sadetta]
    30,  // 289. [sadetta]
    30,  // 290. [sadetta]
    30,  // 291. [sadetta]
    30,  // 292. [sadetta]
    30,  // 293. [sadetta]
    30,  // 294. [sadetta]
    30,  // 295. [sadetta]
    30,  // 296. [sadetta]
    30,  // 297. [sadetta]
    30,  // 298. [sadetta]
    30,  // 299. [sadetta]
    30   // 300. [sadetta]
};

// ----------------------------------------------------------------------
/*!
 * \brief Second table from page \ref page_rain_twoday
 *
 * All 48 unique one day rains mapped onto 17 different reduced
 * cases with two time phrase parameters
 */
// ----------------------------------------------------------------------

rain_dispatch_data two_day_cases[49] = {
    // empty placeholder to get indices to start from 1
    {0, "", ""},
    // 1. Tanaan aamupaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
    // selkeaa]
    {3, "aamupaivasta alkaen", "aamupaivasta alkaen"},
    // 2. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    {4, "aamupaivasta alkaen", ""},
    // 3. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    {2, "aamupaivasta alkaen", "iltapaivasta alkaen"},
    // 4. Tanaan aamupaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
    // selkeaa]
    {3, "aamupaivasta alkaen", "keskipaivasta alkaen"},
    // 5. Tanaan aamupaivasta alkaen [sadetta]
    {1, "aamupaivasta alkaen", ""},
    // 6. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen selkeaa]
    {5, "aamupaivasta alkaen", ""},
    // 7. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    {6, "aamupaivasta alkaen", ""},
    // 8. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
    // selkeaa]
    {3, "aamusta alkaen", "aamupaivasta alkaen"},
    // 9. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    {4, "aamusta alkaen", ""},
    // 10. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    {2, "aamusta alkaen", "iltapaivasta alkaen"},
    // 11. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
    // selkeaa]
    {3, "aamusta alkaen", "keskipaivasta alkaen"},
    // 12. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen selkeaa]
    {5, "aamusta alkaen", ""},
    // 13. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    {6, "aamusta alkaen", ""},
    // 14. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen selkeaa]
    {5, "aamuyosta alkaen", ""},
    // 15. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    {6, "aamuyosta alkaen", ""},
    // 16. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    {7, "illalla", "yolla"},
    // 17. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta alkaen
    // poutaa, [enimmakseen selkeaa]
    {10, "illasta alkaen", "aamupaivasta alkaen"},
    // 18. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta alkaen
    // poutaa
    {9, "illasta alkaen", "iltapaivasta alkaen"},
    // 19. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna keskipaivasta alkaen
    // poutaa, [enimmakseen selkeaa]
    {10, "illasta alkaen", "keskipaivasta alkaen"},
    // 20. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    {8, "illasta alkaen", ""},
    // 21. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    {11, "illasta alkaen", ""},
    // 22. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna aamupaivasta alkaen
    // poutaa, [enimmakseen selkeaa]
    {10, "iltayosta alkaen", "aamupaivasta alkaen"},
    // 23. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta alkaen
    // poutaa
    {9, "iltayosta alkaen", "iltapaivasta alkaen"},
    // 24. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna keskipaivasta alkaen
    // poutaa, [enimmakseen selkeaa]
    {10, "iltayosta alkaen", "keskipaivasta alkaen"},
    // 25. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]
    {8, "iltayosta alkaen", ""},
    // 26. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    {11, "iltayosta alkaen", ""},
    // 27. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    {7, "yolla", ""},
    // 28. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
    // selkeaa]
    {3, "iltapaivasta alkaen", "aamupaivasta alkaen"},
    // 29. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    {4, "iltapaivasta alkaen", ""},
    // 30. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    {2, "iltapaivasta alkaen", "iltapaivasta alkaen"},
    // 31. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
    // selkeaa]
    {3, "iltapaivasta alkaen", "keskipaivasta alkaen"},
    // 32. Tanaan iltapaivasta alkaen [sadetta]
    {1, "iltapaivasta alkaen", ""},
    // 33. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen selkeaa]
    {5, "iltapaivasta alkaen", ""},
    // 34. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    {6, "iltapaivasta alkaen", ""},
    // 35. Tanaan ja huomenna [sadetta]
    {17, "", ""},
    // 36. Tanaan keskipaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
    // selkeaa]
    {3, "keskipaivasta alkaen", "aamupaivasta alkaen"},
    // 37. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    {4, "keskipaivasta alkaen", ""},
    // 38. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    {2, "keskipaivasta alkaen", "iltapaivasta alkaen"},
    // 39. Tanaan keskipaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
    // selkeaa]
    {3, "keskipaivasta alkaen", "keskipaivasta alkaen"},
    // 40. Tanaan keskipaivasta alkaen [sadetta]
    {1, "keskipaivasta alkaen", ""},
    // 41. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
    // selkeaa]
    {5, "keskipaivasta alkaen", ""},
    // 42. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    {6, "keskipaivasta alkaen", ""},
    // 43. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    {15, "aamupaivasta alkaen", ""},
    // 44. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    {15, "aamusta alkaen", ""},
    // 45. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    {16, "", ""},
    // 46. Tanaan [sadetta], huomenna illalla poutaa
    {13, "illalla", ""},
    // 47. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    {14, "iltapaivasta alkaen", ""},
    // 48. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    {15, "keskipaivasta alkaen", ""}};

// ----------------------------------------------------------------------
/*!
 * \brief Table from page \ref page_rain_twoday
 *
 * All possible one day rains mapped onto 48 unique cases
 */
// ----------------------------------------------------------------------

int two_day_forecasts[577] = {
    0,   // place holder to get indices to start from 1
    45,  // 1. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 2. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 3. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 4. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    44,  // 5. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    44,  // 6. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    44,  // 7. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    44,  // 8. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    43,  // 9. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    43,  // 10. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 11. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 12. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    47,  // 13. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 14. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 15. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 16. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    46,  // 17. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 18. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 19. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 20. Tanaan [sadetta], huomenna illalla poutaa
    35,  // 21. Tanaan ja huomenna [sadetta]
    35,  // 22. Tanaan ja huomenna [sadetta]
    35,  // 23. Tanaan ja huomenna [sadetta]
    35,  // 24. Tanaan ja huomenna [sadetta]
    45,  // 25. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 26. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 27. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 28. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    44,  // 29. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    44,  // 30. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    44,  // 31. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    44,  // 32. Tanaan [sadetta], huomenna aamusta alkaen poutaa, [enimmakseen selkeaa]
    43,  // 33. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    43,  // 34. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 35. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 36. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    47,  // 37. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 38. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 39. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 40. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    46,  // 41. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 42. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 43. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 44. Tanaan [sadetta], huomenna illalla poutaa
    35,  // 45. Tanaan ja huomenna [sadetta]
    35,  // 46. Tanaan ja huomenna [sadetta]
    35,  // 47. Tanaan ja huomenna [sadetta]
    35,  // 48. Tanaan ja huomenna [sadetta]
    45,  // 49. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 50. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 51. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 52. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    14,  // 53. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 54. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 55. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 56. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    43,  // 57. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    43,  // 58. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 59. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 60. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    47,  // 61. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 62. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 63. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 64. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    46,  // 65. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 66. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 67. Tanaan [sadetta], huomenna illalla poutaa
    46,  // 68. Tanaan [sadetta], huomenna illalla poutaa
    35,  // 69. Tanaan ja huomenna [sadetta]
    35,  // 70. Tanaan ja huomenna [sadetta]
    35,  // 71. Tanaan ja huomenna [sadetta]
    35,  // 72. Tanaan ja huomenna [sadetta]
    45,  // 73. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 74. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 75. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 76. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    14,  // 77. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 78. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 79. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 80. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    43,  // 81. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    43,  // 82. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 83. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 84. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    47,  // 85. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 86. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 87. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 88. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    15,  // 89. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 90. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 91. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 92. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 93. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 94. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    35,  // 95. Tanaan ja huomenna [sadetta]
    35,  // 96. Tanaan ja huomenna [sadetta]
    45,  // 97. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 98. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 99. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    45,  // 100. Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    14,  // 101. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 102. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 103. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    14,  // 104. Tanaan aamuyosta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    43,  // 105. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    43,  // 106. Tanaan [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 107. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    48,  // 108. Tanaan [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen selkeaa]
    47,  // 109. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 110. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 111. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    47,  // 112. Tanaan [sadetta], huomenna iltapaivasta alkaen poutaa
    15,  // 113. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 114. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 115. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 116. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 117. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    15,  // 118. Tanaan aamuyosta alkaen tiistai-iltaan asti [sadetta]
    35,  // 119. Tanaan ja huomenna [sadetta]
    35,  // 120. Tanaan ja huomenna [sadetta]
    9,   //  121. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  122. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  123. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  124. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    12,  // 125. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 126. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 127. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 128. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    8,   //  129. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
         //  selkeaa]
    8,   //  130. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
         //  selkeaa]
    11,  // 131. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
         // selkeaa]
    11,  // 132. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
         // selkeaa]
    10,  // 133. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 134. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 135. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 136. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    13,  // 137. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 138. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 139. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 140. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 141. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 142. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    35,  // 143. Tanaan ja huomenna [sadetta]
    35,  // 144. Tanaan ja huomenna [sadetta]
    9,   //  145. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  146. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  147. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  148. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    12,  // 149. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 150. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 151. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 152. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    8,   //  153. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
         //  selkeaa]
    8,   //  154. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
         //  selkeaa]
    11,  // 155. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
         // selkeaa]
    11,  // 156. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
         // selkeaa]
    10,  // 157. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 158. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 159. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    13,  // 160. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 161. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 162. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 163. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 164. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 165. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 166. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    35,  // 167. Tanaan ja huomenna [sadetta]
    35,  // 168. Tanaan ja huomenna [sadetta]
    9,   //  169. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  170. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  171. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  172. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    12,  // 173. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 174. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 175. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 176. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    8,   //  177. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
         //  selkeaa]
    8,   //  178. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
         //  selkeaa]
    11,  // 179. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
         // selkeaa]
    11,  // 180. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
         // selkeaa]
    10,  // 181. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 182. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 183. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 184. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    13,  // 185. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 186. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 187. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 188. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 189. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 190. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    35,  // 191. Tanaan ja huomenna [sadetta]
    35,  // 192. Tanaan ja huomenna [sadetta]
    9,   //  193. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  194. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  195. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    9,   //  196. Tanaan aamusta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    12,  // 197. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 198. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 199. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    12,  // 200. Tanaan aamusta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    8,   //  201. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
         //  selkeaa]
    8,   //  202. Tanaan aamusta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa, [enimmakseen
         //  selkeaa]
    11,  // 203. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
         // selkeaa]
    11,  // 204. Tanaan aamusta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa, [enimmakseen
         // selkeaa]
    10,  // 205. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 206. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 207. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    10,  // 208. Tanaan aamusta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    13,  // 209. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 210. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 211. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 212. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 213. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    13,  // 214. Tanaan aamusta alkaen tiistai-iltaan asti [sadetta]
    35,  // 215. Tanaan ja huomenna [sadetta]
    35,  // 216. Tanaan ja huomenna [sadetta]
    2,   //  217. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    2,   //  218. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    2,   //  219. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    2,   //  220. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    6,   //  221. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         //  selkeaa]
    6,   //  222. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         //  selkeaa]
    6,   //  223. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         //  selkeaa]
    6,   //  224. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         //  selkeaa]
    1,   //  225. Tanaan aamupaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         //  [enimmakseen selkeaa]
    1,   //  226. Tanaan aamupaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         //  [enimmakseen selkeaa]
    4,   //  227. Tanaan aamupaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         //  [enimmakseen selkeaa]
    4,   //  228. Tanaan aamupaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         //  [enimmakseen selkeaa]
    3,   //  229. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    3,   //  230. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    3,   //  231. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    3,   //  232. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    7,   //  233. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  234. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  235. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  236. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  237. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  238. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    5,   //  239. Tanaan aamupaivasta alkaen [sadetta]
    5,   //  240. Tanaan aamupaivasta alkaen [sadetta]
    2,   //  241. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    2,   //  242. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    2,   //  243. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    2,   //  244. Tanaan aamupaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    6,   //  245. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         //  selkeaa]
    6,   //  246. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         //  selkeaa]
    6,   //  247. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         //  selkeaa]
    6,   //  248. Tanaan aamupaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         //  selkeaa]
    1,   //  249. Tanaan aamupaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         //  [enimmakseen selkeaa]
    1,   //  250. Tanaan aamupaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         //  [enimmakseen selkeaa]
    4,   //  251. Tanaan aamupaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         //  [enimmakseen selkeaa]
    4,   //  252. Tanaan aamupaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         //  [enimmakseen selkeaa]
    3,   //  253. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    3,   //  254. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    3,   //  255. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    3,   //  256. Tanaan aamupaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    7,   //  257. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  258. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  259. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  260. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  261. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    7,   //  262. Tanaan aamupaivasta alkaen tiistai-iltaan asti [sadetta]
    5,   //  263. Tanaan aamupaivasta alkaen [sadetta]
    5,   //  264. Tanaan aamupaivasta alkaen [sadetta]
    37,  // 265. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    37,  // 266. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    37,  // 267. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    37,  // 268. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    41,  // 269. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    41,  // 270. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    41,  // 271. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    41,  // 272. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    36,  // 273. Tanaan keskipaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    36,  // 274. Tanaan keskipaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    39,  // 275. Tanaan keskipaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    39,  // 276. Tanaan keskipaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    38,  // 277. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    38,  // 278. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    38,  // 279. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    38,  // 280. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    42,  // 281. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 282. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 283. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 284. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 285. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 286. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    40,  // 287. Tanaan keskipaivasta alkaen [sadetta]
    40,  // 288. Tanaan keskipaivasta alkaen [sadetta]
    37,  // 289. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    37,  // 290. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    37,  // 291. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    37,  // 292. Tanaan keskipaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    41,  // 293. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    41,  // 294. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    41,  // 295. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    41,  // 296. Tanaan keskipaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    36,  // 297. Tanaan keskipaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    36,  // 298. Tanaan keskipaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    39,  // 299. Tanaan keskipaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    39,  // 300. Tanaan keskipaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    38,  // 301. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    38,  // 302. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    38,  // 303. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    38,  // 304. Tanaan keskipaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    42,  // 305. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 306. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 307. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 308. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 309. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    42,  // 310. Tanaan keskipaivasta alkaen tiistai-iltaan asti [sadetta]
    40,  // 311. Tanaan keskipaivasta alkaen [sadetta]
    40,  // 312. Tanaan keskipaivasta alkaen [sadetta]
    29,  // 313. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 314. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 315. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 316. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    33,  // 317. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 318. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 319. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 320. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    28,  // 321. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    28,  // 322. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 323. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 324. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    30,  // 325. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 326. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 327. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 328. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    34,  // 329. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 330. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 331. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 332. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 333. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 334. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    40,  // 335. Tanaan keskipaivasta alkaen [sadetta]
    40,  // 336. Tanaan keskipaivasta alkaen [sadetta]
    29,  // 337. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 338. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 339. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 340. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    33,  // 341. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 342. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 343. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 344. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    28,  // 345. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    28,  // 346. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 347. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 348. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    30,  // 349. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 350. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 351. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 352. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    34,  // 353. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 354. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 355. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 356. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 357. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 358. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    40,  // 359. Tanaan keskipaivasta alkaen [sadetta]
    40,  // 360. Tanaan keskipaivasta alkaen [sadetta]
    29,  // 361. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 362. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 363. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 364. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    33,  // 365. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 366. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 367. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 368. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    28,  // 369. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    28,  // 370. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 371. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 372. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    30,  // 373. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 374. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 375. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 376. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    34,  // 377. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 378. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 379. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 380. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 381. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 382. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    32,  // 383. Tanaan iltapaivasta alkaen [sadetta]
    32,  // 384. Tanaan iltapaivasta alkaen [sadetta]
    29,  // 385. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 386. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 387. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 388. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    33,  // 389. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 390. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 391. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 392. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    28,  // 393. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    28,  // 394. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 395. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 396. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    30,  // 397. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 398. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 399. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 400. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    34,  // 401. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 402. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 403. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 404. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 405. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 406. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    32,  // 407. Tanaan iltapaivasta alkaen [sadetta]
    32,  // 408. Tanaan iltapaivasta alkaen [sadetta]
    29,  // 409. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 410. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 411. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    29,  // 412. Tanaan iltapaivasta alkaen [sadetta], huomenna [enimmakseen selkeaa]
    33,  // 413. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 414. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 415. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    33,  // 416. Tanaan iltapaivasta alkaen tiistai-aamuun asti [sadetta], tiistaina [enimmakseen
         // selkeaa]
    28,  // 417. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    28,  // 418. Tanaan iltapaivasta alkaen [sadetta], huomenna aamupaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 419. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    31,  // 420. Tanaan iltapaivasta alkaen [sadetta], huomenna keskipaivasta alkaen poutaa,
         // [enimmakseen selkeaa]
    30,  // 421. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 422. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 423. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    30,  // 424. Tanaan iltapaivasta alkaen [sadetta], huomenna iltapaivasta alkaen poutaa
    34,  // 425. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 426. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 427. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 428. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 429. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    34,  // 430. Tanaan iltapaivasta alkaen tiistai-iltaan asti [sadetta]
    32,  // 431. Tanaan iltapaivasta alkaen [sadetta]
    32,  // 432. Tanaan iltapaivasta alkaen [sadetta]
    16,  // 433. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 434. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 435. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 436. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 437. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 438. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 439. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 440. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    17,  // 441. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    17,  // 442. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    19,  // 443. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    19,  // 444. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    18,  // 445. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 446. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 447. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 448. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    21,  // 449. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 450. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 451. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 452. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 453. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 454. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    20,  // 455. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    20,  // 456. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    16,  // 457. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 458. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 459. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 460. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 461. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 462. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 463. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 464. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    17,  // 465. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    17,  // 466. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    19,  // 467. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    19,  // 468. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    18,  // 469. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 470. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 471. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 472. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    21,  // 473. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 474. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 475. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 476. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 477. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 478. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    20,  // 479. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    20,  // 480. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    16,  // 481. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 482. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 483. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 484. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 485. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 486. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 487. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 488. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    17,  // 489. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    17,  // 490. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    19,  // 491. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    17,  // 492. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    18,  // 493. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 494. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 495. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 496. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    21,  // 497. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 498. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 499. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 500. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 501. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 502. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    20,  // 503. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    20,  // 504. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    16,  // 505. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 506. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 507. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 508. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 509. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 510. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 511. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    16,  // 512. Tanaan [enimmakseen selkeaa], illalla ja yolla [sadetta]. Huomenna [enimmakseen
         // selkeaa]
    17,  // 513. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    17,  // 514. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    19,  // 515. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    19,  // 516. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    18,  // 517. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 518. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 519. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    18,  // 520. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    21,  // 521. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 522. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 523. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 524. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 525. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    21,  // 526. Tanaan [enimmakseen selkeaa], illasta alkaen tiistai-iltaan asti [sadetta]
    20,  // 527. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    20,  // 528. Tanaan [enimmakseen selkeaa], illasta alkaen [sadetta]
    27,  // 529. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 530. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 531. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 532. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 533. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 534. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 535. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 536. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    22,  // 537. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    22,  // 538. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    24,  // 539. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    24,  // 540. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    23,  // 541. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    23,  // 542. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    23,  // 543. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    23,  // 544. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    26,  // 545. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 546. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 547. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 548. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 549. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 550. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    25,  // 551. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]
    25,  // 552. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]
    27,  // 553. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 554. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 555. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 556. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 557. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 558. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 559. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    27,  // 560. Tanaan [enimmakseen selkeaa], yolla [sadetta]. Huomenna [enimmakseen selkeaa]
    22,  // 561. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    22,  // 562. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna aamupaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    24,  // 563. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    24,  // 564. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna keskipaivasta
         // alkaen poutaa, [enimmakseen selkeaa]
    23,  // 565. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    23,  // 566. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    23,  // 567. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    23,  // 568. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]. Huomenna iltapaivasta
         // alkaen poutaa
    26,  // 569. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 570. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 571. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 572. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 573. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    26,  // 574. Tanaan [enimmakseen selkeaa], iltayosta alkaen tiistai-iltaan asti [sadetta]
    25,  // 575. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]
    25   // 576. Tanaan [enimmakseen selkeaa], iltayosta alkaen [sadetta]
};

// ----------------------------------------------------------------------
/*!
 * \brief Calculates index for rain in the first table at \ref page_rain_oneday
 *
 * If forecast length is N+1, the start index for that forecast length is
 * the arithmetic sum of 24+23+...+24-N+1. The sum is then (a1+an)*n/2
 * where a1=24, an=24-n+1. The final index is then obtained by adding
 * the starthour, plus one since the indices start at 1.
 *
 * For example, if the rain spans 4-6:
 *  -# n = (6-4)-1 = 1
 *  -# a1 = 24
 *  -# an = 24-n+1 = 24
 *  -# sn = ((a1+an)*n)/2 = ((24+24)*2)/2 = 24
 *  -# 24+4+1 = 29, the final result
 *
 * For example, if the rain spans 5-18:
 *  -# n = (18-5)-1 = 12
 *  -# a1 = 24
 *  -# an = 24-n+1 = 13
 *  -# sn = ((a1+an)*n)/2 = ((24+13)*12)/2 = 222
 *  -# 222+5+1 = 228, the final result
 *
 * \param theStartHour The start hour of the rain
 * \param theEndHour The end hour of the rain
 * \return The index for the rain
 */
// ----------------------------------------------------------------------

int one_day_rain_index(int theStartHour, int theEndHour)
{
  if (theEndHour <= theStartHour)
    throw TextGen::TextGenError(
        "Internal error in weather_overview: end hour must be greater than start hour");

  const int n = theEndHour - theStartHour - 1;
  const int a1 = 24;
  const int an = 24 - n + 1;
  const int sn = ((a1 + an) * n) / 2;
  return (sn + theStartHour + 1);
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculates index for rain in the second table at \ref page_rain_oneday
 *
 * \param theStartHour The start hour of the rain
 * \param theEndHour The end hour of the rain
 * \return The index for the rain
 */
// ----------------------------------------------------------------------

int one_day_rain_unique_index(int theStartHour, int theEndHour)
{
  const int tmp = one_day_rain_index(theStartHour, theEndHour);
  return one_day_forecasts[tmp];
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculates index for rain in the first table at \ref page_rain_twoday
 *
 * \param theStartHour The start hour of the rain
 * \param theEndHour The end hour of the rain
 * \return The index for the rain
 */
// ----------------------------------------------------------------------

int two_day_rain_index(int theStartHour, int theEndHour)
{
  return (theStartHour * 24 + theEndHour);
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculates index for rain in the second table at \ref page_rain_twoday
 *
 * \param theStartHour The start hour of the rain
 * \param theEndHour The end hour of the rain
 * \return The index for the rain
 */
// ----------------------------------------------------------------------

int two_day_rain_unique_index(int theStartHour, int theEndHour)
{
  const int tmp = two_day_rain_index(theStartHour, theEndHour);
  return two_day_forecasts[tmp];
}
}

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generator story on a day with a single inclusive rain
 */
// ----------------------------------------------------------------------

Sentence one_inclusive_rain(const TextGenPosixTime& theForecastTime,
                            const AnalysisSources& theSources,
                            const WeatherArea& theArea,
                            const WeatherPeriod& thePeriod,
                            const string& theVar,
                            const WeatherPeriod& theRainPeriod,
                            int theDay)
{
  using namespace CloudinessStoryTools;
  using PrecipitationStoryTools::rain_phrase;

  Sentence s;
  s << PeriodPhraseFactory::create("days", theVar, theForecastTime, thePeriod);

  // start & end times of the rain
  int rainstarthour = theRainPeriod.localStartTime().GetHour();
  int rainendhour = theRainPeriod.localEndTime().GetHour() + 1;
  if (rainendhour == 1) rainendhour = 24;

  // mapping onto the four basic cases
  const int idx = one_day_rain_unique_index(rainstarthour, rainendhour);
  const int phrase = one_day_cases[idx].index;

  switch (phrase)
  {
    // [Aamulla] [sadetta]
    case 1:
    {
      s << one_day_cases[idx].phrase1
        << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      break;
    }
    // [Aamulla] [sadetta], [aamulla] [enimmakseen selkeaa] ja poutaa
    case 2:
    {
      WeatherPeriod cloudyperiod(theRainPeriod.localEndTime(), thePeriod.localEndTime());
      s << one_day_cases[idx].phrase1
        << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
        << one_day_cases[idx].phrase2
        << cloudiness_phrase(theSources, theArea, cloudyperiod, theVar, theDay) << "ja"
        << "poutaa";
      break;
    }
    // [Aamulla] [sadetta], [aamulla] poutaa
    case 3:
    {
      s << one_day_cases[idx].phrase1
        << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
        << one_day_cases[idx].phrase2 << "poutaa";
      break;
    }
    // [Enimmakseen selkeaa], [aamulla] [sadetta]
    case 4:
    {
      s << cloudiness_phrase(theSources, theArea, thePeriod, theVar, theDay) << Delimiter(",")
        << one_day_cases[idx].phrase1
        << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      break;
    }
    default:
      throw TextGenError("Internal error in weather_overview::one_inclusive_rain");
  }
  return s;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generator story on two days with one inclusive rain
 */
// ----------------------------------------------------------------------

Paragraph one_twoday_inclusive_rain(const TextGenPosixTime& theForecastTime,
                                    const AnalysisSources& theSources,
                                    const WeatherArea& theArea,
                                    const WeatherPeriod& thePeriod,
                                    const string& theVar,
                                    const WeatherPeriod& theRainPeriod,
                                    int theDay)
{
  using CloudinessStoryTools::cloudiness_phrase;
  using PrecipitationStoryTools::rain_phrase;

  // start & end times of the rain
  int rainstarthour = theRainPeriod.localStartTime().GetHour();
  int rainendhour = theRainPeriod.localEndTime().GetHour() + 1;
  if (rainendhour == 1) rainendhour = 24;

  // mapping onto the seventeen basic cases
  const int idx = two_day_rain_unique_index(rainstarthour, rainendhour);
  const int phrase = two_day_cases[idx].index;
  const char* phrase1 = two_day_cases[idx].phrase1;
  const char* phrase2 = two_day_cases[idx].phrase2;

  // days 1 and 2
  const WeatherPeriod day1(TimeTools::dayStart(theRainPeriod.localStartTime()),
                           TimeTools::dayEnd(theRainPeriod.localStartTime()));
  const WeatherPeriod day2(TimeTools::nextDay(day1.localStartTime()),
                           TimeTools::nextDay(day1.localEndTime()));

  // the period before the rain
  const TextGenPosixTime before_rain_start(TimeTools::dayStart(theRainPeriod.localStartTime()));
  const TextGenPosixTime before_rain_end(theRainPeriod.localStartTime());
  const WeatherPeriod before_rain(before_rain_start, before_rain_end);

  // the period after the rain
  const TextGenPosixTime after_rain_start(TimeTools::addHours(theRainPeriod.localEndTime(), 1));
  const TextGenPosixTime after_rain_end(TimeTools::dayEnd(after_rain_start));
  const WeatherPeriod after_rain(after_rain_start, after_rain_end);

  Paragraph paragraph;
  Sentence s1;
  Sentence s2;
  switch (phrase)
  {
    // Tanaan [aamusta alkaen] [sadetta]
    case 1:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1) << phrase1
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      break;
    }
    // Tanaan [aamusta alkaen] [sadetta], huomenna [aamusta alkaen] poutaa
    case 2:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1) << phrase1
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
         << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2) << phrase2
         << "poutaa";
      break;
    }
    // Tanaan [aamusta alkaen] [sadetta], huomenna [aamusta alkaen] poutaa, [enimmakseen selkeaa]
    case 3:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1) << phrase1
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
         << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2) << phrase2
         << "poutaa" << Delimiter(",")
         << cloudiness_phrase(theSources, theArea, after_rain, theVar, theDay + 1);
      break;
    }
    // Tanaan [aamusta alkaen] [sadetta], huomenna [enimmakseen selkeaa]
    case 4:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1) << phrase1
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
         << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2)
         << cloudiness_phrase(theSources, theArea, after_rain, theVar, theDay + 1);
      break;
    }
    // Tanaan [aamusta alkaen] tiistai-aamuun asti [sadetta], tiistaina [enimmakseen selkeaa]
    case 5:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1) << phrase1
         << WeekdayTools::until_weekday_morning(day2.localStartTime()) << "asti"
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
         << WeekdayTools::on_weekday(day2.localStartTime())
         << cloudiness_phrase(theSources, theArea, after_rain, theVar, theDay + 1);
      break;
    }
    // Tanaan [aamusta alkaen] tiistai-iltaan asti [sadetta]
    case 6:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1) << phrase1
         << WeekdayTools::until_weekday_evening(day2.localStartTime()) << "asti"
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      break;
    }
    // Tanaan [enimmakseen selkeaa], [aamulla] ja [yolla] [sadetta]. Huomenna [enimmakseen selkeaa]
    case 7:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << cloudiness_phrase(theSources, theArea, before_rain, theVar, theDay) << Delimiter(",")
         << phrase1;
      if (strcmp(phrase2, "") != 0) s1 << "ja" << phrase2;
      s1 << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);

      s2 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2)
         << cloudiness_phrase(theSources, theArea, after_rain, theVar, theDay + 1);
      break;
    }
    // Tanaan [enimmakseen selkeaa], [aamusta alkaen] [sadetta]
    case 8:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << cloudiness_phrase(theSources, theArea, before_rain, theVar, theDay) << Delimiter(",")
         << phrase1 << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      break;
    }
    // Tanaan [enimmakseen selkeaa], [aamusta alkaen] [sadetta]. Huomenna [aamusta alkaen] poutaa
    case 9:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << cloudiness_phrase(theSources, theArea, before_rain, theVar, theDay) << Delimiter(",")
         << phrase1 << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      s2 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2) << phrase2
         << "poutaa";
      break;
    }
    // Tanaan [enimmakseen selkeaa], [aamusta alkaen] [sadetta]. Huomenna [aamusta alkaen] poutaa,
    // [enimmakseen selkeaa]
    case 10:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << cloudiness_phrase(theSources, theArea, before_rain, theVar, theDay) << Delimiter(",")
         << phrase1 << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      s2 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2) << phrase2
         << "poutaa" << Delimiter(",")
         << cloudiness_phrase(theSources, theArea, after_rain, theVar, theDay + 1);
      break;
    }
    // Tanaan [enimmakseen selkeaa], [aamusta alkaen] tiistai-iltaan asti [sadetta]
    case 11:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << cloudiness_phrase(theSources, theArea, before_rain, theVar, theDay) << Delimiter(",")
         << phrase1 << WeekdayTools::until_weekday_evening(day2.localStartTime()) << "asti"
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      break;
    }
    // Tanaan [enimmakseen selkeaa], [aamulla] [sadetta]. Huomenna [enimmakseen selkeaa]
    case 12:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << cloudiness_phrase(theSources, theArea, before_rain, theVar, theDay) << Delimiter(",")
         << phrase1 << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      s2 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2)
         << cloudiness_phrase(theSources, theArea, after_rain, theVar, theDay + 1);
      break;
    }
    // Tanaan [sadetta], huomenna [aamulla] poutaa
    case 13:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
         << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2) << phrase2
         << "poutaa";
      break;
    }
    // Tanaan [sadetta], huomenna [aamusta alkaen] poutaa
    case 14:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
         << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2) << phrase2
         << "poutaa";
      break;
    }
    // Tanaan [sadetta], huomenna [aamusta alkaen] poutaa, [enimmakseen selkeaa]
    case 15:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay) << Delimiter(",")
         << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2) << phrase2
         << "poutaa" << Delimiter(",")
         << cloudiness_phrase(theSources, theArea, after_rain, theVar, theDay + 1);
      break;
    }
    // Tanaan [sadetta], huomenna [enimmakseen selkeaa]
    case 16:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1)
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay + 1) << Delimiter(",")
         << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2) << phrase2
         << cloudiness_phrase(theSources, theArea, after_rain, theVar, theDay + 1);
      break;
    }
    // Tanaan ja huomenna [sadetta]
    case 17:
    {
      s1 << PeriodPhraseFactory::create("today", theVar, theForecastTime, day1) << "ja"
         << PeriodPhraseFactory::create("today", theVar, theForecastTime, day2)
         << rain_phrase(theSources, theArea, theRainPeriod, theVar, theDay);
      break;
    }
    default:
      throw TextGenError("Internal error in weather_overview::one_twoday_inclusive_rain");
  }

  paragraph << s1 << s2;
  return paragraph;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate story on a day with many inclusive rains
 */
// ----------------------------------------------------------------------

Sentence many_inclusive_rains(const TextGenPosixTime& theForecastTime,
                              const AnalysisSources& theSources,
                              const WeatherArea& theArea,
                              const WeatherPeriod& thePeriod,
                              const string& theVar,
                              const PrecipitationPeriodTools::RainPeriods& theRainPeriods)
{
  Sentence s;
  s << PeriodPhraseFactory::create("days", theVar, theForecastTime, thePeriod);
  s << "enimmakseen"
    << "poutaa";
  return s;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate overview on weather
 *
 * \return The story
 *
 * \see page_weather_overview
 *
 * \todo Much missing
 */
// ----------------------------------------------------------------------

Paragraph WeatherStory::overview() const
{
  MessageLogger log("WeatherStory::overview");

  using namespace PrecipitationPeriodTools;

  Paragraph paragraph;

  // we want the last day to extend up to midnight regardless
  // of the actual period length, otherwise we risk badly
  // formed forecasts when using full-day descriptions. If the end
  // hour is too early, we ignore the last day fully. Also, we do not
  // want to analyze any times before the first period, which we'd
  // get if we used itsPeriod start time as the start time. Hence we
  // need some trickery...

  // extended end time
  const TextGenPosixTime endtime =
      (itsPeriod.localEndTime().GetHour() < 12 ? TimeTools::dayStart(itsPeriod.localEndTime())
                                               : TimeTools::dayEnd(itsPeriod.localEndTime()));

  // valid generator for all days
  HourPeriodGenerator generator(WeatherPeriod(itsPeriod.localStartTime(), endtime),
                                itsVar + "::day");

  // minimal inclusive period
  WeatherPeriod rainperiod = generator.period();

  RainPeriods rainperiods =
      PrecipitationPeriodTools::analyze(itsSources, itsArea, rainperiod, itsVar);

  log << "Found " << rainperiods.size() << " rainy periods" << endl;

  {
    for (RainPeriods::const_iterator it = rainperiods.begin(); it != rainperiods.end(); it++)
    {
      log << "Period: " << it->localStartTime() << " ... " << it->localEndTime() << endl;
    }
  }

  if (rainperiods.size() == 0)
  {
    CloudinessStory story(itsForecastTime, itsSources, itsArea, itsPeriod, itsVar);
    paragraph << story.makeStory("cloudiness_overview");
    log << paragraph;
    return paragraph;
  }

  // process sequences of similar days

  const int n = generator.size();

  RainPeriods dummy;
  vector<RainPeriods> overlaps;
  vector<RainPeriods> inclusives;
  overlaps.push_back(dummy);
  inclusives.push_back(dummy);

  for (int day = 1; day <= n; day++)
  {
    WeatherPeriod period = generator.period(day);

    RainPeriods overlap = overlappingPeriods(rainperiods, period);
    RainPeriods inclusive = inclusivePeriods(rainperiods, period);

    overlaps.push_back(overlap);
    inclusives.push_back(inclusive);
  }

  for (int day = 1; day <= n; day++)
  {
    const RainPeriods::size_type noverlap = overlaps[day].size();
    const RainPeriods::size_type ninclusive = inclusives[day].size();

    log << "Day " << day << " overlap   : " << noverlap << endl;
    log << "Day " << day << " inclusive : " << ninclusive << endl;

    if (noverlap == 0)
    {
      // find sequence of non-rainy days, report all at once
      int day2 = day;
      for (; day2 < n; day2++)
      {
        if (overlaps[day2 + 1].size() != 0 || inclusives[day2 + 1].size() != 0) break;
      }

      WeatherPeriod period(generator.period(day).localStartTime(),
                           generator.period(day2).localEndTime());

      if (day != day2)
        log << "Cloudiness only for days " << day << '-' << day2 << endl;
      else
        log << "Cloudiness only for day " << day << endl;

      CloudinessStory story(itsForecastTime, itsSources, itsArea, period, itsVar);

      paragraph << story.makeStory("cloudiness_overview");
      day = day2;
    }

    else if (ninclusive == 1 && noverlap == 1)
    {
      paragraph << one_inclusive_rain(itsForecastTime,
                                      itsSources,
                                      itsArea,
                                      generator.period(day),
                                      itsVar,
                                      inclusives[day].front(),
                                      day);
    }
    else if (ninclusive == noverlap)
    {
      paragraph << many_inclusive_rains(
          itsForecastTime, itsSources, itsArea, generator.period(day), itsVar, inclusives[day]);
    }
    else if (ninclusive == 0 && noverlap == 1 && day + 1 <= n && overlaps[day + 1].size() == 1 &&
             inclusives[day + 1].size() == 0)
    {
      WeatherPeriod period(generator.period(day).localStartTime(),
                           generator.period(day + 1).localEndTime());
      paragraph << one_twoday_inclusive_rain(
          itsForecastTime, itsSources, itsArea, period, itsVar, overlaps[day].front(), day);
      day++;
    }
    else
    {
      // seek end of rainy days
      // find sequence of non-rainy days, report all at once
      int day2 = day;
      for (; day2 < n; day2++)
      {
        // found end if there is a non-rainy day
        if (overlaps[day2 + 1].size() == 0) break;
        // found end if there is a 1-rain inclusive day
        if (overlaps[day2 + 1].size() == 1 && inclusives[day2 + 1].size() == 1) break;
      }

      WeatherPeriod period(generator.period(day).localStartTime(),
                           generator.period(day2).localEndTime());

      Sentence s;
      if (day == day2)
      {
        s << PeriodPhraseFactory::create("today", itsVar, itsForecastTime, period) << "enimmakseen"
          << "poutaa";
      }
      else
      {
        s << WeekdayTools::from_weekday(period.localStartTime()) << "ajoittain sadetta";
      }
      paragraph << s;
      day = day2;
    }
  }

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
