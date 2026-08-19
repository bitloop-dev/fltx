# Ubuntu Mono report font

The WOFF2 files in this directory are report-specific subsets of Ubuntu Mono
Regular and Bold from Canonical's official Ubuntu Font Family 0.83 package:

https://design.ubuntu.com/font

They contain printable ASCII and the small set of symbols used by the metrics
SVG reports:

`U+0020-007E,U+00B7,U+00D7,U+2013-2014,U+2212,U+221E,U+2264-2265,U+2713`

The subsets are embedded into compact overview SVGs so their layout and
appearance do not depend on locally installed fonts. Ubuntu Mono is
distributed under the Ubuntu Font Licence 1.0; see `LICENCE.txt`.
