/**
 * fltx/detail/pow_tables.h - Shared exact power lookup table storage.
 *
 * Generated from exact integer and rational decompositions.
 */

#ifndef FLTX_DETAIL_POW_TABLES_INCLUDED
#define FLTX_DETAIL_POW_TABLES_INCLUDED
#include <bit>
#include <cstdint>

namespace bl::detail {

struct pow_table_entry
{
    double x0;
    double x1;
    double x2;
    double x3;
};

namespace pow_tables {

inline constexpr int pow10_min_exponent = -323;
inline constexpr int pow10_max_exponent = 308;

inline constexpr pow_table_entry pow10_table[] = {
        pow_table_entry{  0x0.0000000000002p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-323
        pow_table_entry{  0x0.0000000000014p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-322
        pow_table_entry{  0x0.00000000000cap-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-321
        pow_table_entry{  0x0.00000000007e8p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-320
        pow_table_entry{  0x0.0000000004f10p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-319
        pow_table_entry{  0x0.00000000316a2p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-318
        pow_table_entry{  0x0.00000001ee257p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-317
        pow_table_entry{  0x0.000000134d761p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-316
        pow_table_entry{  0x0.000000c1069cdp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-315
        pow_table_entry{  0x0.0000078a42205p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-314
        pow_table_entry{  0x0.00004b6695433p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-313
        pow_table_entry{  0x0.0002f201d49fbp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-312
        pow_table_entry{  0x0.001d74124e3d1p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-311
        pow_table_entry{  0x0.012688b70e62bp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-310
        pow_table_entry{  0x0.0b8157268fdafp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-309
        pow_table_entry{  0x0.730d67819e8d2p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-308
        pow_table_entry{  0x1.1fa182c40c60ep-1020, -0x0.0000000000002p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-307
        pow_table_entry{  0x1.6789e3750f791p-1017, -0x0.0000000000006p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-306
        pow_table_entry{  0x1.c16c5c5253575p-1014, 0x0.0000000000008p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-305
        pow_table_entry{  0x1.18e3b9b374169p-1010, 0x0.000000000024bp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-304
        pow_table_entry{  0x1.5f1ca820511c3p-1007, 0x0.00000000036f0p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-303
        pow_table_entry{  0x1.b6e3d22865634p-1004, 0x0.0000000012565p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-302
        pow_table_entry{  0x1.124e63593f5e1p-1000, -0x0.0000000148a0ep-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-301
        pow_table_entry{  0x1.56e1fc2f8f359p-997, -0x0.00000004d6491p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-300
        pow_table_entry{  0x1.ac9a7b3b7302fp-994, 0x0.0000000fa1259p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-299
        pow_table_entry{  0x1.0be08d0527e1dp-990, 0x0.0000069c4b77fp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-298
        pow_table_entry{  0x1.4ed8b04671da5p-987, -0x0.00001de50d50dp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-297
        pow_table_entry{  0x1.a28edc580e50ep-984, -0x0.00002af285285p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-296
        pow_table_entry{  0x1.059949b708f29p-980, -0x0.0011ad7933930p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-295
        pow_table_entry{  0x1.46ff9c24cb2f3p-977, -0x0.0030c6bc03be1p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-294
        pow_table_entry{  0x1.98bf832dfdfb0p-974, -0x0.05e7c358256cbp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-293
        pow_table_entry{  0x1.feef63f97d79cp-971, -0x0.3b0da171763ecp-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-292
        pow_table_entry{  0x1.3f559e7bee6c1p-967, 0x1.b177b191618c5p-1022, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e-291
        pow_table_entry{  0x1.8f2b061aea072p-964, -0x1.f115310523085p-1018, 0x0.0000000000005p-1022, 0x0.0000000000000p+0  }, // 1e-290
        pow_table_entry{  0x1.f2f5c7a1a488ep-961, -0x1.b569f519af297p-1017, -0x0.000000000000dp-1022, 0x0.0000000000000p+0  }, // 1e-289
        pow_table_entry{  0x1.37d99cc506d59p-957, -0x1.44588e4c035e8p-1011, 0x0.00000000002bbp-1022, 0x0.0000000000000p+0  }, // 1e-288
        pow_table_entry{  0x1.85d003f6488afp-954, -0x1.2add63be086c3p-1009, -0x0.00000000004b2p-1022, 0x0.0000000000000p+0  }, // 1e-287
        pow_table_entry{  0x1.e74404f3daadbp-951, -0x1.baca5e56c543ap-1005, 0x0.0000000001111p-1022, 0x0.0000000000000p+0  }, // 1e-286
        pow_table_entry{  0x1.308a831868ac9p-947, -0x1.94be7af63b4a4p-1001, -0x0.0000000075556p-1022, 0x0.0000000000000p+0  }, // 1e-285
        pow_table_entry{  0x1.7cad23de82d7bp-944, -0x1.f3dc33679439bp-999, 0x0.000000036aaa0p-1022, 0x0.0000000000000p+0  }, // 1e-284
        pow_table_entry{  0x1.dbd86cd6238d9p-941, 0x1.c7965fdf435bfp-995, 0x0.000000322aa3cp-1022, 0x0.0000000000000p+0  }, // 1e-283
        pow_table_entry{  0x1.29674405d6388p-937, -0x1.8d081051d79a2p-993, 0x0.000000f5aa655p-1022, 0x0.0000000000000p+0  }, // 1e-282
        pow_table_entry{  0x1.73c115074bc6ap-934, -0x1.f04a14664d80ap-990, 0x0.00000198a7f4ep-1022, 0x0.0000000000000p+0  }, // 1e-281
        pow_table_entry{  0x1.d0b15a491eb84p-931, 0x1.64e8d9a007c7dp-985, -0x0.00003009706efp-1022, 0x0.0000000000000p+0  }, // 1e-280
        pow_table_entry{  0x1.226ed86db3333p-927, -0x1.20ee77fbfb232p-981, 0x0.00021fa19baaap-1022, 0x0.0000000000000p+0  }, // 1e-279
        pow_table_entry{  0x1.6b0a8e891ffffp-924, 0x1.96d5ea0506142p-978, -0x0.006ac3afeb55ep-1022, 0x0.0000000000000p+0  }, // 1e-278
        pow_table_entry{  0x1.c5cd322b67fffp-921, 0x1.f916c90c8f324p-976, -0x0.002ba4df315b0p-1022, 0x0.0000000000000p+0  }, // 1e-277
        pow_table_entry{  0x1.1ba03f5b21000p-917, -0x1.e228e12c13405p-971, 0x0.1e4b8f4812724p-1022, 0x0.0000000000000p+0  }, // 1e-276
        pow_table_entry{  0x1.62884f31e93ffp-914, 0x1.a54ce688e7efap-968, 0x0.2ef398d0b8769p-1022, 0x0.0000000000000p+0  }, // 1e-275
        pow_table_entry{  0x1.bb2a62fe638ffp-911, 0x1.0ea0202b21eb9p-965, -0x1.c54f80fb196bcp-1019, -0x0.0000000000002p-1022  }, // 1e-274
        pow_table_entry{  0x1.14fa7ddefe3a0p-907, -0x1.d6dbebe50accdp-961, 0x1.64ae4f63101cap-1015, 0x0.000000000002ep-1022  }, // 1e-273
        pow_table_entry{  0x1.5a391d56bdc87p-904, 0x1.b36d1921b2800p-958, 0x1.7bb3c677a847ap-1013, -0x0.0000000000033p-1022  }, // 1e-272
        pow_table_entry{  0x1.b0c764ac6d3a9p-901, 0x1.20485f6a1f200p-955, 0x1.daa0b81592598p-1010, 0x0.0000000000604p-1022  }, // 1e-271
        pow_table_entry{  0x1.0e7c9eebc444ap-897, -0x1.97a588bb59180p-952, 0x1.28a4730d7b77fp-1006, 0x0.0000000003c26p-1022  }, // 1e-270
        pow_table_entry{  0x1.521bc6a6b555cp-894, 0x1.01388a8ae8510p-948, 0x1.72cd8fd0da55fp-1003, 0x0.0000000005980p-1022  }, // 1e-269
        pow_table_entry{  0x1.a6a2b85062ab3p-891, 0x1.4186ad2da2654p-945, 0x1.cf80f3c510eb7p-1000, -0x0.00000000c8104p-1022  }, // 1e-268
        pow_table_entry{  0x1.0825b3323dab0p-887, 0x1.23d0b0f215fd3p-943, -0x1.bc9ecf49aad9bp-997, -0x0.0000000fd0a2bp-1022  }, // 1e-267
        pow_table_entry{  0x1.4a2f1ffecd15cp-884, 0x1.6cc4dd2e9b7c7p-940, 0x1.a872f9c7d4dfbp-995, 0x0.00000021d9a51p-1022  }, // 1e-266
        pow_table_entry{  0x1.9cbae7fe805b3p-881, 0x1.c7f6147a425b9p-937, 0x1.28fb839ca17a1p-996, 0x0.000000128072ap-1022  }, // 1e-265
        pow_table_entry{  0x1.01f4d0ff10390p-877, -0x1.c60c66672d0d9p-934, 0x1.0b99d3241e4ecp-988, 0x0.00001339047a5p-1022  }, // 1e-264
        pow_table_entry{  0x1.4272053ed4474p-874, -0x1.1bc7c0007c287p-930, -0x1.d8bfdc096d0ecp-984, -0x0.00013fc5d338ap-1022  }, // 1e-263
        pow_table_entry{  0x1.930e868e89591p-871, -0x1.62b9b0009b329p-927, -0x1.4eefd30bc8527p-981, -0x0.000c7dba40368p-1022  }, // 1e-262
        pow_table_entry{  0x1.f7d228322baf5p-868, 0x1.224bf1ff9f006p-923, 0x1.5d5438314598fp-978, -0x0.003ce94682212p-1022  }, // 1e-261
        pow_table_entry{  0x1.3ae3591f5b4d9p-864, 0x1.b56f773fc3604p-919, -0x1.25ab5ce134807p-974, 0x0.039ee33eeab4cp-1022  }, // 1e-260
        pow_table_entry{  0x1.899c2f6732210p-861, -0x1.ee9a557825e3ep-915, 0x1.a43a72f99f97ep-969, -0x0.3bcb1f8ad4f05p-1022  }, // 1e-259
        pow_table_entry{  0x1.ec033b40fea93p-858, 0x1.95bf1529d0a33p-912, 0x1.a921f700efbabp-971, 0x0.2a10c493ae9cbp-1022  }, // 1e-258
        pow_table_entry{  0x1.338205089f29cp-854, 0x1.f65db4e889980p-910, -0x1.dec958b3ed457p-964, 0x1.8a4a7adc4d21fp-1018  }, // 1e-257
        pow_table_entry{  0x1.8062864ac6f43p-851, 0x1.39fa911155ff0p-906, -0x1.2b3dd770744b6p-960, -0x1.1322e66c9f959p-1015  }, // 1e-256
        pow_table_entry{  0x1.e07b27dd78b14p-848, -0x1.de1b2aa952051p-905, -0x1.d83535324578fp-959, 0x1.5028bff0710a1p-1013  }, // 1e-255
        pow_table_entry{  0x1.2c4cf8ea6b6ecp-844, 0x1.daa5e0aac597ap-898, -0x1.74e42827ed6d7p-952, -0x1.2b79a2026e567p-1007  }, // 1e-254
        pow_table_entry{  0x1.77603725064a8p-841, -0x1.aeb0a72a89028p-895, 0x1.6f166e70b9b99p-952, -0x1.d9602a0c27b02p-1006  }, // 1e-253
        pow_table_entry{  0x1.d53844ee47dd1p-838, 0x1.e5a32f0ad4bcep-892, 0x1.cadc0a0ce827fp-949, -0x1.4fb8348f319c2p-1003  }, // 1e-252
        pow_table_entry{  0x1.25432b14ecea3p-834, -0x1.41e80a64ec27dp-890, 0x1.1ec986481118fp-945, 0x1.5c59be4d01fcdp-1000  }, // 1e-251
        pow_table_entry{  0x1.6e93f5da2824cp-831, -0x1.6498833f89cc7p-885, -0x1.3308304bd541ap-943, -0x1.323f487ef60ffp-999  }, // 1e-250
        pow_table_entry{  0x1.ca38f350b22dfp-828, -0x1.bdbea40f6c3f9p-882, 0x1.d006b87426adcp-937, -0x1.2fd9e353d6728p-993  }, // 1e-249
        pow_table_entry{  0x1.1e6398126f5cbp-824, 0x1.a5a365d971612p-880, 0x1.10219a44c164cp-936, -0x1.7bd05c28cc0f2p-990  }, // 1e-248
        pow_table_entry{  0x1.65fc7e170b33ep-821, -0x1.f0f3c0b032469p-877, -0x1.aaf57fca83908p-931, -0x1.76b11cccbfc4cp-985  }, // 1e-247
        pow_table_entry{  0x1.bf7b9d9cce00dp-818, 0x1.64b3d3c8f049fp-872, 0x1.d49a4085b716bp-929, 0x1.5d14e0008250cp-985  }, // 1e-246
        pow_table_entry{  0x1.17ad428200c08p-814, 0x1.5ef0645d962e3p-868, 0x1.a49c0d0a724dcp-922, 0x1.76d16860028b9p-976  }, // 1e-245
        pow_table_entry{  0x1.5d98932280f0ap-811, 0x1.b6ac7d74fbb9cp-865, 0x1.0dc3104d0ee13p-919, 0x1.d485c278032e8p-973  }, // 1e-244
        pow_table_entry{  0x1.b4feb7eb212cdp-808, 0x1.22bce691d541bp-865, -0x1.76615cfd6b33dp-919, -0x1.b2c6674fe02f5p-973  }, // 1e-243
        pow_table_entry{  0x1.111f32f2f4bc0p-804, 0x1.2d6d8406c9524p-859, 0x1.4b0192f0ce7fdp-914, -0x1.8fbc0091ec1d9p-969  }, // 1e-242
        pow_table_entry{  0x1.5566ffafb1eb0p-801, 0x1.78c8e5087ba6dp-856, 0x1.9dc1f7ad021fcp-911, 0x1.8a9fe9331b61fp-971  }, // 1e-241
        pow_table_entry{  0x1.aac0bf9b9e65cp-798, 0x1.d6fb1e4a9a909p-853, -0x1.fd66c533deac2p-907, -0x1.fc257039003b9p-961  }, // 1e-240
        pow_table_entry{  0x1.0ab877c142ffap-794, -0x1.6cd18688afb2dp-848, -0x1.5f301da03595dp-902, 0x1.c26899dc5fdadp-957  }, // 1e-239
        pow_table_entry{  0x1.4d6695b193bf8p-791, 0x1.bfd0bea92303bp-848, -0x1.b7e1284217da0p-902, 0x1.9816029bbe8bep-957  }, // 1e-238
        pow_table_entry{  0x1.a0c03b1df8af6p-788, 0x1.17e27729b5e25p-844, -0x1.92ecb9294ee84p-898, 0x1.fe1b8342ae2edp-954  }, // 1e-237
        pow_table_entry{  0x1.047824f2bb6dap-784, -0x1.a8893ac2f7295p-839, 0x1.c216062317577p-893, -0x1.b04bb37d94c8bp-948  }, // 1e-236
        pow_table_entry{  0x1.45962e2f6a490p-781, 0x1.ed54768c4b0c6p-836, 0x1.329b87abdd2d4p-890, 0x1.f1d0afd183029p-944  }, // 1e-235
        pow_table_entry{  0x1.96fbb9bb44db4p-778, 0x1.3454ca17aee7cp-832, -0x1.017b2cd2570edp-888, 0x1.b9136f178f0cep-943  }, // 1e-234
        pow_table_entry{  0x1.fcbaa82a16121p-775, 0x1.8169fc9d9aa1bp-829, -0x1.41d9f806ecd28p-885, 0x1.3ac256eb96809p-943  }, // 1e-233
        pow_table_entry{  0x1.3df4a91a4dcb5p-771, -0x1.1e3b843afeb5ep-826, -0x1.64941d822a01cp-880, -0x1.f9da344d660f8p-934  }, // 1e-232
        pow_table_entry{  0x1.8d71d360e13e2p-768, 0x1.346b356c83394p-824, 0x1.091b6c752df72p-879, -0x1.e1430582fe4d7p-933  }, // 1e-231
        pow_table_entry{  0x1.f0ce4839198dbp-765, -0x1.9f9e7f4e16fe2p-819, 0x1.296c48f24f2eap-873, -0x1.0b3278dc77bc2p-927  }, // 1e-230
        pow_table_entry{  0x1.3680ed23aff89p-761, -0x1.83c30f90ce5edp-815, -0x1.187149a23a0b8p-871, 0x1.6401d1d8d4a9cp-925  }, // 1e-229
        pow_table_entry{  0x1.8421286c9bf6bp-758, -0x1.c967a6ea03ed1p-813, 0x1.50b931fa9bb8dp-867, 0x1.bd02464f09d43p-922  }, // 1e-228
        pow_table_entry{  0x1.e5297287c2f45p-755, 0x1.e21f37adbd8bep-809, -0x1.ad8c40c35eac8p-863, 0x1.0b10b5f8b3125p-917  }, // 1e-227
        pow_table_entry{  0x1.2f39e794d9d8bp-751, 0x1.ad5382cc96776p-805, 0x1.f3885785e4d43p-859, 0x1.4dd4e376dfd6ep-914  }, // 1e-226
        pow_table_entry{  0x1.7b08617a104eep-748, 0x1.18a8637fbc154p-802, 0x1.c1a9b59d78250p-858, -0x1.7ad78eada0cd9p-913  }, // 1e-225
        pow_table_entry{  0x1.d9ca79d89462ap-745, -0x1.425b0740a9caep-800, 0x1.190a11826b172p-854, -0x1.d98d72590900fp-910  }, // 1e-224
        pow_table_entry{  0x1.281e8c275cbdap-741, 0x1.36871b7795e13p-796, 0x1.afa64af182ee7p-850, 0x1.6c03cc442d2fbp-905  }, // 1e-223
        pow_table_entry{  0x1.72262f3133ed1p-738, -0x1.3deb8ed542534p-792, 0x1.1b8fddade3aa1p-847, -0x1.c7da05563c230p-905  }, // 1e-222
        pow_table_entry{  0x1.ceafbafd80e85p-735, -0x1.1acce51525d02p-790, 0x1.6273d5195c949p-844, 0x1.b8c5ef2a869a8p-899  }, // 1e-221
        pow_table_entry{  0x1.212dd4de70913p-731, 0x1.3cffc34b2177cp-788, -0x1.13bcd68131192p-843, 0x1.37bb57a942094p-899  }, // 1e-220
        pow_table_entry{  0x1.69794a160cb58p-728, -0x1.9cf012f8858a9p-783, -0x1.2b1581842fabfp-837, 0x1.985aa2d93928cp-892  }, // 1e-219
        pow_table_entry{  0x1.c3d79c9b8fe2ep-725, -0x1.02160bdb5376ap-779, 0x1.8a251e1ac4691p-834, 0x1.ff38a5c7c3b97p-888  }, // 1e-218
        pow_table_entry{  0x1.1a66c1e139eddp-721, -0x1.a14dc769142a2p-775, -0x1.09a8cd2f453e5p-830, -0x1.01f2618c96b06p-886  }, // 1e-217
        pow_table_entry{  0x1.6100725988694p-718, -0x1.09a139435934bp-772, 0x1.59f67fc274b91p-826, -0x1.509bbe7bef172p-881  }, // 1e-216
        pow_table_entry{  0x1.b9408eefea839p-715, -0x1.4c0987942f81dp-769, -0x1.4f8be04cee18bp-823, 0x1.6cf54794548c7p-880  }, // 1e-215
        pow_table_entry{  0x1.13c85955f2923p-711, 0x1.b07a0b43624eep-765, -0x1.51b76c3014cf7p-819, 0x1.3906532f2d35fp-874  }, // 1e-214
        pow_table_entry{  0x1.58ba6fab6f36cp-708, 0x1.1c988e143ae29p-762, 0x1.676ae30f97f2ep-818, -0x1.e2e060141df25p-873  }, // 1e-213
        pow_table_entry{  0x1.aee90b964b047p-705, 0x1.63beb199499b3p-759, 0x1.705166f4df7bep-813, 0x1.348cf0fcdb522p-867  }, // 1e-212
        pow_table_entry{  0x1.0d51a73deee2dp-701, -0x1.a1a8d10031ff0p-755, 0x1.98cb81642eb5cp-811, -0x1.f93f4b0fb7655p-866  }, // 1e-211
        pow_table_entry{  0x1.50a6110d6a9b8p-698, -0x1.0a1305403e7ecp-752, 0x1.fefe61bd3a633p-808, -0x1.3bc78ee9d29f5p-862  }, // 1e-210
        pow_table_entry{  0x1.a4cf9550c5426p-695, -0x1.4c97c6904e1e7p-749, 0x1.3f5efd16447e0p-804, -0x1.455cb95223a39p-858  }, // 1e-209
        pow_table_entry{  0x1.0701bd527b498p-691, -0x1.cfdedc1a30d30p-745, -0x1.1c3250e90a98ap-799, -0x1.96b3e7a6ac8c7p-855  }, // 1e-208
        pow_table_entry{  0x1.48c22ca71a1bdp-688, 0x1.bc296cdf42f84p-742, -0x1.633ee5234d3edp-796, 0x1.80e7c79bea142p-850  }, // 1e-207
        pow_table_entry{  0x1.9af2b7d0e0a2dp-685, -0x1.a9986fd1d8937p-740, 0x1.0fc5864f7dc61p-795, -0x1.ede467d1b66dep-851  }, // 1e-206
        pow_table_entry{  0x1.00d7b2e28c65cp-681, -0x1.3fe8bc64eb849p-741, -0x1.6248c0e516437p-795, -0x1.a576071890255p-850  }, // 1e-205
        pow_table_entry{  0x1.410d9f9b2f7f3p-678, -0x1.8fe2eb7e2665cp-738, 0x1.45250ee1a42bbp-792, -0x1.da711bd685d53p-852  }, // 1e-204
        pow_table_entry{  0x1.91510781fb5f0p-675, -0x1.07cf6e9976c00p-729, 0x1.acb37294d069bp-784, 0x1.37b5de53a67b1p-838  }, // 1e-203
        pow_table_entry{  0x1.f5a549627a36cp-672, -0x1.49c34a3fd4700p-726, 0x1.0bf0279d02421p-780, 0x1.0b46abd12033cp-836  }, // 1e-202
        pow_table_entry{  0x1.39874ddd8c623p-668, 0x1.31e5f1981b3a0p-722, 0x1.4eec318442d29p-777, 0x1.538615b15a103p-831  }, // 1e-201
        pow_table_entry{  0x1.87e92154ef7acp-665, 0x1.f97db7f888221p-721, -0x1.7563086ab1e31p-776, -0x1.5e6193893daf3p-830  }, // 1e-200
        pow_table_entry{  0x1.e9e369aa2b597p-662, 0x1.3bee92fb55155p-717, -0x1.f4aef2a15796fp-771, -0x1.ad7e7e1ae346cp-825  }, // 1e-199
        pow_table_entry{  0x1.322e220a5b17ep-658, 0x1.e2ba8dee8a96ap-712, 0x1.a389542d94a0dp-766, 0x1.7390f12f31f3dp-821  }, // 1e-198
        pow_table_entry{  0x1.7eb9aa8cf1ddep-655, 0x1.6da4c5a8b4f14p-711, 0x1.8d75271f3920fp-768, 0x1.0752d7afe70bbp-822  }, // 1e-197
        pow_table_entry{  0x1.de6815302e556p-652, -0x1.8dbc823b4774ap-706, 0x1.0f869387383b5p-760, -0x1.9db6c39320f99p-814  }, // 1e-196
        pow_table_entry{  0x1.2b010d3e1cf56p-648, -0x1.f895d1650ca8ep-702, -0x1.592f8f2df36bdp-758, 0x1.f5b717102d902p-812  }, // 1e-195
        pow_table_entry{  0x1.75c1508da432bp-645, -0x1.daed16f93f4c6p-701, -0x1.af7b72f97046cp-755, 0x1.7324dcd438f43p-809  }, // 1e-194
        pow_table_entry{  0x1.d331a4b10d3f6p-642, -0x1.946a172de3c7ep-696, -0x1.b5a4fb7cc5869p-756, 0x1.fdc28128e6271p-811  }, // 1e-193
        pow_table_entry{  0x1.23ff06eea847ap-638, -0x1.fcc24e7cae5cfp-692, 0x1.f773c71690246p-747, -0x1.782ccde8ce04fp-804  }, // 1e-192
        pow_table_entry{  0x1.6cfec8aa52598p-635, -0x1.efcb886f67d0ap-691, 0x1.d542e370d0b5ep-746, -0x1.d638016301863p-801  }, // 1e-191
        pow_table_entry{  0x1.c83e7ad4e6efep-632, -0x1.35df3545a0e26p-687, -0x1.b56c63b2fb1cbp-743, 0x1.b439fe443e184p-798  }, // 1e-190
        pow_table_entry{  0x1.1d270cc51055fp-628, -0x1.60d5c0a5c246cp-682, 0x1.774e20d811871p-738, -0x1.7bd6f045564c3p-792  }, // 1e-189
        pow_table_entry{  0x1.6470cff6546b6p-625, 0x1.46f4cf30cd279p-679, 0x1.d521a90e15e8dp-735, -0x1.b59958ad57be8p-790  }, // 1e-188
        pow_table_entry{  0x1.bd8d03f3e9864p-622, -0x1.9d37f40bfe3a2p-678, -0x1.b595ecae649d0p-732, -0x1.17fd76c56d713p-790  }, // 1e-187
        pow_table_entry{  0x1.1678227871f3ep-618, 0x1.bf6f41de2046fp-672, -0x1.845f6cfb3fb89p-726, 0x1.fd440657126e6p-780  }, // 1e-186
        pow_table_entry{  0x1.5c162b168e70ep-615, 0x1.7a5892ad42c52p-672, 0x1.a88b7c5f05956p-727, -0x1.b57c09947b00ep-784  }, // 1e-185
        pow_table_entry{  0x1.b31bb5dc320d2p-612, -0x1.c4e22914ed913p-666, -0x1.3daa34912720bp-721, 0x1.b77493d019990p-775  }, // 1e-184
        pow_table_entry{  0x1.0ff151a99f483p-608, -0x1.b0d59ad147ac0p-666, 0x1.cbacf92a3c5cbp-720, 0x1.2a8dc620fff9dp-775  }, // 1e-183
        pow_table_entry{  0x1.53eda614071a4p-605, -0x1.21d0b01859997p-659, 0x1.1f4c1bba65b9fp-716, -0x1.159d90ad800f7p-773  }, // 1e-182
        pow_table_entry{  0x1.a8e90f9908e0dp-602, -0x1.6a44dc1e6fffdp-656, 0x1.2ce3e4551fe51p-710, -0x1.4ad827a6c700ap-765  }, // 1e-181
        pow_table_entry{  0x1.0991a9bfa58c8p-598, -0x1.89ac264c17ff8p-654, 0x1.e07375a99f794p-709, 0x1.89c739be1cfd0p-764  }, // 1e-180
        pow_table_entry{  0x1.4bf6142f8eefap-595, -0x1.ec172fdf1dff6p-651, 0x1.2c48298a03abdp-705, -0x1.84f1bdf496f0fp-759  }, // 1e-179
        pow_table_entry{  0x1.9ef3993b72ab8p-592, 0x1.6638c10a46a03p-646, 0x1.bbad19f6424b6p-701, -0x1.cc5c5ae3795a6p-757  }, // 1e-178
        pow_table_entry{  0x1.03583fc527ab3p-588, 0x1.bfc6f14cd8484p-643, 0x1.54c3039e96f1bp-701, -0x1.fb9b8ce2bd879p-757  }, // 1e-177
        pow_table_entry{  0x1.442e4fb671960p-585, 0x1.7dc56d0072d28p-643, 0x1.a9f3c4863cae2p-698, -0x1.9ea09c06db3a6p-752  }, // 1e-176
        pow_table_entry{  0x1.9539e3a40dfb8p-582, 0x1.dd36c8408f872p-640, 0x1.0a385ad3e5ecdp-694, -0x1.9230c224823ccp-755  }, // 1e-175
        pow_table_entry{  0x1.fa885c8d117a6p-579, 0x1.2a423d2859b47p-636, 0x1.a66338c46fb40p-690, 0x1.f8250c354974dp-746  }, // 1e-174
        pow_table_entry{  0x1.3c9539d82aec8p-575, -0x1.d165a671b1fbdp-630, 0x1.43ff01bd62e84p-685, 0x1.3b1727a14de90p-742  }, // 1e-173
        pow_table_entry{  0x1.8bba884e35a7ap-572, -0x1.22df88070f3d6p-626, -0x1.ac04f74d1176cp-684, 0x1.89dcf189a1634p-739  }, // 1e-172
        pow_table_entry{  0x1.eea92a61c3118p-569, 0x1.28d12bee59e69p-624, -0x1.0b831a902aea3p-680, -0x1.84eaf484fd910p-734  }, // 1e-171
        pow_table_entry{  0x1.3529ba7d19eafp-565, 0x1.730576e9f0603p-621, 0x1.58ce0f65e52dap-676, -0x1.cc4b634c79ea7p-732  }, // 1e-170
        pow_table_entry{  0x1.8274291c6065bp-562, -0x1.181c95adc9c3ep-617, -0x1.43f9b302861bfp-675, 0x1.c0a1c3e0679afp-729  }, // 1e-169
        pow_table_entry{  0x1.e3113363787f2p-559, -0x1.af11dd8c9e1a7p-613, 0x1.e6b07e03cd85dp-668, 0x1.cc328d3620607p-724  }, // 1e-168
        pow_table_entry{  0x1.2deac01e2b4f7p-555, -0x1.ad654efc5a107p-614, -0x1.fa3627b3f18bap-669, 0x1.f9f9841d43c43p-724  }, // 1e-167
        pow_table_entry{  0x1.79657025b6235p-552, -0x1.10c5f515db84ap-606, -0x1.21e30ec683b7cp-660, 0x1.7cf0efca49297p-714  }, // 1e-166
        pow_table_entry{  0x1.d7becc2f23ac2p-549, -0x1.53ddc96d49973p-605, -0x1.a96f49e09296ap-659, -0x1.1e96a2192461dp-714  }, // 1e-165
        pow_table_entry{  0x1.26d73f9d764b9p-545, 0x1.95cab10dd900cp-600, -0x1.13cb1c58b73c5p-656, 0x1.4ce1dab04942ep-710  }, // 1e-164
        pow_table_entry{  0x1.708d0f84d3de7p-542, 0x1.fd9eaea8a7a07p-596, 0x1.d4e84392235e9p-650, 0x1.14034a2b8b727p-704  }, // 1e-163
        pow_table_entry{  0x1.ccb0536608d61p-539, 0x1.7d065a52d1889p-593, 0x1.4a225476ac364p-647, -0x1.a6fbe34991b0fp-701  }, // 1e-162
        pow_table_entry{  0x1.1fee341fc585dp-535, -0x1.23b80f187a154p-590, -0x1.6355166ba8bc4p-644, 0x1.ef4523e409e2dp-698  }, // 1e-161
        pow_table_entry{  0x1.67e9c127b6e74p-532, 0x1.26b3da42cecadp-588, 0x1.0f568fe5b452ep-643, 0x1.ac59b374316e2p-697  }, // 1e-160
        pow_table_entry{  0x1.c1e43171a4a11p-529, 0x1.7060d0d3827d8p-585, 0x1.a99619ef90b3dp-639, 0x1.77020513dc9a1p-698  }, // 1e-159
        pow_table_entry{  0x1.192e9ee706e4bp-525, -0x1.4670df5ef39c6p-579, -0x1.7b0117e522c7dp-634, 0x1.0ea61432c69e0p-690  }, // 1e-158
        pow_table_entry{  0x1.5f7a46a0c89ddp-522, 0x1.67f2e8c94f7c8p-576, 0x1.131f5110ca432p-630, -0x1.56d8336043dd4p-686  }, // 1e-157
        pow_table_entry{  0x1.b758d848fac55p-519, -0x1.3e105d045ca46p-573, 0x1.57e72554fcd3ep-627, 0x1.94dc6ff1eacaep-681  }, // 1e-156
        pow_table_entry{  0x1.1297872d9cbb5p-515, -0x1.1b28e88ae79aep-571, -0x1.487c45570fdc8p-626, -0x1.7b1d0466a09b0p-684  }, // 1e-155
        pow_table_entry{  0x1.573d68f903ea2p-512, 0x1.4f066ea92f3f3p-567, 0x1.32b254a996163p-622, -0x1.d9e4458048c1cp-681  }, // 1e-154
        pow_table_entry{  0x1.ad0cc33744e4bp-509, -0x1.2e9bfad642788p-563, -0x1.01422c5808c89p-620, 0x1.dafa2a91fa50ep-674  }, // 1e-153
        pow_table_entry{  0x1.0c27fa028b0efp-505, -0x1.3d217cc5e98b5p-559, -0x1.4192b76e0afabp-617, 0x1.51b8b53678e51p-671  }, // 1e-152
        pow_table_entry{  0x1.4f31f8832dd2ap-502, 0x1.739624089c11ep-556, -0x1.191f765498db9p-610, -0x1.559d91d7be8e2p-664  }, // 1e-151
        pow_table_entry{  0x1.a2fe76a3f9475p-499, -0x1.7c2297a9e74d7p-556, 0x1.04c560b2076c3p-610, -0x1.5827b26d718d0p-664  }, // 1e-150
        pow_table_entry{  0x1.05df0a267bcc9p-495, 0x1.8935309ae7b7dp-551, -0x1.ba09472176b8dp-607, 0x1.51ce60f7320fbp-661  }, // 1e-149
        pow_table_entry{  0x1.4756ccb01abfbp-492, 0x1.7ae09f3068697p-546, 0x1.d77467162b990p-604, 0x1.4c83f269fd274p-659  }, // 1e-148
        pow_table_entry{  0x1.992c7fdc216fap-489, 0x1.b3318df90507ap-544, -0x1.b655cfe489301p-598, -0x1.e605b10fb838fp-652  }, // 1e-147
        pow_table_entry{  0x1.ff779fd329cb9p-486, -0x1.e0020e88b9b68p-541, -0x1.1f5a1eed5be0fp-598, 0x1.03c71562cdc6bp-652  }, // 1e-146
        pow_table_entry{  0x1.3faac3e3fa1f3p-482, 0x1.e9ff5b7545f6fp-536, 0x1.f4c67acaba693p-590, 0x1.b225c6d5dc09cp-644  }, // 1e-145
        pow_table_entry{  0x1.8f9574dcf8a70p-479, 0x1.647f32529774bp-533, 0x1.71f8197d69038p-587, 0x1.1eaf388b530c3p-641  }, // 1e-144
        pow_table_entry{  0x1.f37ad21436d0cp-476, 0x1.bd9efee73d51ep-530, 0x1.9cec3fb98688dp-585, -0x1.3349f2a3b0618p-639  }, // 1e-143
        pow_table_entry{  0x1.382cc34ca2428p-472, -0x1.d2f9415ef359ap-527, -0x1.fbd8b05817d50p-582, -0x1.0038de9938f3cp-637  }, // 1e-142
        pow_table_entry{  0x1.8637f41fcad32p-469, -0x1.23dbc8db58180p-523, -0x1.9eb3b71b87729p-577, -0x1.4047163f8730bp-634  }, // 1e-141
        pow_table_entry{  0x1.e7c5f127bd87ep-466, 0x1.265a89dba3c3fp-521, -0x1.9829389a53cd3p-580, -0x1.058dbcf68fcd8p-635  }, // 1e-140
        pow_table_entry{  0x1.30dbb6b8d674fp-462, -0x1.480769d6b9a59p-517, 0x1.780731e4fc5d0p-571, -0x1.fa378961a19e0p-627  }, // 1e-139
        pow_table_entry{  0x1.7d12a4670c123p-459, -0x1.cd04a22634077p-513, -0x1.94fb80d0e245ep-567, -0x1.3c62b5dd0502cp-623  }, // 1e-138
        pow_table_entry{  0x1.dc574d80cf16bp-456, 0x1.7f746aa07ded6p-511, -0x1.f474c20a35aebp-565, -0x1.8b7b635446437p-620  }, // 1e-137
        pow_table_entry{  0x1.29b69070816e3p-452, -0x1.0573d5bb14ba9p-511, 0x1.cdc1ae679cb40p-567, 0x1.1a5c3d6a82badp-621  }, // 1e-136
        pow_table_entry{  0x1.7424348ca1c9cp-449, -0x1.0a3686594ecf5p-503, 0x1.9c8264340307cp-557, 0x1.0583cd33148dap-612  }, // 1e-135
        pow_table_entry{  0x1.d12d41afca3c3p-446, -0x1.4cc427efa2832p-500, 0x1.03a2fd4103c9bp-554, 0x1.46e4c07fd9b11p-609  }, // 1e-134
        pow_table_entry{  0x1.22bc490dde65ap-442, -0x1.4ffa98f5c591fp-496, -0x1.76e886dd7687cp-552, -0x1.9d883d80bf8abp-608  }, // 1e-133
        pow_table_entry{  0x1.6b6b5b5155ff0p-439, 0x1.701b033324265p-495, -0x1.d4a2a894d429bp-549, -0x1.0275267077b6bp-604  }, // 1e-132
        pow_table_entry{  0x1.c6463225ab7ecp-436, 0x1.cc21c3ffed2fep-492, -0x1.49cb52ba09342p-546, 0x1.79db1fe6d4b75p-602  }, // 1e-131
        pow_table_entry{  0x1.1bebdf578b2f4p-432, -0x1.b81ab96002f08p-486, -0x1.7387c4ed11702p-540, -0x1.313d70c0fbb0dp-594  }, // 1e-130
        pow_table_entry{  0x1.62e6d72d6dfb0p-429, 0x1.d9de9847fc536p-483, -0x1.d069b62855cc3p-537, 0x1.04e6661d8ac5ep-592  }, // 1e-129
        pow_table_entry{  0x1.bba08cf8c979dp-426, -0x1.afa9c1a60497dp-480, -0x1.12108ec9acfcep-536, -0x1.73c000b625114p-590  }, // 1e-128
        pow_table_entry{  0x1.1544581b7dec2p-422, -0x1.1b94320f85bdcp-477, -0x1.55a52c9f060f0p-531, -0x1.f42c0038eb956p-585  }, // 1e-127
        pow_table_entry{  0x1.5a956e225d672p-419, 0x1.4ec360b64c696p-473, 0x1.2a78c41c9c36ap-527, -0x1.389b8023933d6p-581  }, // 1e-126
        pow_table_entry{  0x1.b13ac9aaf4c0fp-416, -0x1.762f1c7081f11p-472, 0x1.d45bd48f0d110p-526, 0x1.e4f67f4e1fcd2p-580  }, // 1e-125
        pow_table_entry{  0x1.0ec4be0ad8f89p-412, 0x1.4588a38e6bb25p-466, 0x1.a92e59365a0abp-520, -0x1.b4397c1bcb07fp-574  }, // 1e-124
        pow_table_entry{  0x1.5275ed8d8f36cp-409, -0x1.6915338df9611p-463, -0x1.d90c20f81ee56p-518, 0x1.bd7049ba846c2p-572  }, // 1e-123
        pow_table_entry{  0x1.a71368f0f3047p-406, -0x1.c35a807177b96p-460, 0x1.d8586b64ecb0bp-514, -0x1.e999d1eb6d3c7p-568  }, // 1e-122
        pow_table_entry{  0x1.086c219697e2cp-402, 0x1.979dbee454b0ap-458, 0x1.39ba18f89f735p-513, -0x1.90011999222e1p-567  }, // 1e-121
        pow_table_entry{  0x1.4a8729fc3ddb7p-399, 0x1.fd852e9d69dcdp-455, -0x1.3bebb0649c57fp-509, -0x1.e802bffed5733p-565  }, // 1e-120
        pow_table_entry{  0x1.9d28f47b4d525p-396, -0x1.831985bb3bac0p-452, -0x1.15cd38fb86dbep-507, 0x1.9dfc900175301p-562  }, // 1e-119
        pow_table_entry{  0x1.023998cd10537p-392, 0x1.0e100c6afab48p-448, -0x1.5b40873a6892dp-504, -0x1.fa844bfe2d83fp-559  }, // 1e-118
        pow_table_entry{  0x1.42c7ff0054685p-389, -0x1.5735f83d234f3p-444, -0x1.b210a90902b79p-501, 0x1.c36d5081238d8p-555  }, // 1e-117
        pow_table_entry{  0x1.9379fec069826p-386, 0x1.4bf226ce4f741p-443, -0x1.0f4a69a5a1b2bp-497, -0x1.65dbadaf49c79p-551  }, // 1e-116
        pow_table_entry{  0x1.f8587e7083e30p-383, -0x1.cc2229efc395ep-437, 0x1.d6717df87af05p-493, -0x1.7ea532363872ep-549  }, // 1e-115
        pow_table_entry{  0x1.3b374f06526dep-379, -0x1.1f955a35da3dbp-433, 0x1.4981bbaed3359p-487, -0x1.dde4e7ec3c690p-542  }, // 1e-114
        pow_table_entry{  0x1.8a0522c7e7095p-376, 0x1.310a9e795e65dp-431, 0x1.37c455351005ep-485, -0x1.5578879d2e0cep-541  }, // 1e-113
        pow_table_entry{  0x1.ec866b79e0cbap-373, 0x1.bea6a30bdaffap-427, 0x1.42dab5412a03bp-481, -0x1.355ad5308f320p-535  }, // 1e-112
        pow_table_entry{  0x1.33d4032c2c7f5p-369, -0x1.e8d7da1897204p-423, 0x1.c9c8b148ba425p-477, -0x1.4158c53e597f4p-531  }, // 1e-111
        pow_table_entry{  0x1.80c903f7379f2p-366, -0x1.630dd09ebce84p-420, -0x1.c3c52265172d2p-474, -0x1.235ded1bdfbe2p-529  }, // 1e-110
        pow_table_entry{  0x1.e0fb44f50586ep-363, 0x1.10baece64f76ap-419, -0x1.a5b357f2e7c35p-474, -0x1.b0d5a18b5eb6bp-528  }, // 1e-109
        pow_table_entry{  0x1.2c9d0b1923745p-359, -0x1.aac595f8072afp-414, 0x1.f0dfd2105e4bdp-471, 0x1.c5e9ec2393374p-526  }, // 1e-108
        pow_table_entry{  0x1.77c44ddf6c516p-356, -0x1.576fb7608f5abp-415, 0x1.b45f1a51d77b2p-470, 0x1.bb233963c0284p-526  }, // 1e-107
        pow_table_entry{  0x1.d5b561574765bp-353, 0x1.f295a2d63a667p-407, 0x1.5085db8399356p-461, 0x1.ea29ec07bcb03p-515  }, // 1e-106
        pow_table_entry{  0x1.25915cd68c9f9p-349, 0x1.6f3b0b8bc9001p-404, 0x1.494ea4c8ff058p-459, 0x1.92d19c26af710p-514  }, // 1e-105
        pow_table_entry{  0x1.6ef5b40c2fc77p-346, 0x1.e584e7375da01p-400, -0x1.19176c81304e4p-454, -0x1.c10f3f99f4966p-508  }, // 1e-104
        pow_table_entry{  0x1.cab3210f3bb95p-343, 0x1.5ee6210535081p-397, -0x1.7d751e85f1876p-453, -0x1.8a987c038ddf7p-508  }, // 1e-103
        pow_table_entry{  0x1.1eaff4a98553dp-339, 0x1.5b4fd4a341251p-393, -0x1.bb9a4cc4edbd2p-447, -0x1.ded3e9b047157p-501  }, // 1e-102
        pow_table_entry{  0x1.665bf1d3e6a8dp-336, -0x1.4ddc3633ee91bp-390, -0x1.2a80dff6292c7p-444, -0x1.5a239071636b5p-500  }, // 1e-101
        pow_table_entry{  0x1.bff2ee48e0530p-333, -0x1.42a68781d46c4p-388, -0x1.d4845fcecdde3p-443, -0x1.b0ac748dbc462p-497  }, // 1e-100
        pow_table_entry{  0x1.17f7d4ed8c33ep-329, -0x1.9350296249875p-385, -0x1.24d2bbe140aaep-439, -0x1.1cd791b12b57ap-494  }, // 1e-99
        pow_table_entry{  0x1.5df5ca28ef40dp-326, 0x1.81f6f3114905bp-380, 0x1.647e25499bcaap-434, -0x1.ac81aec3aec5bp-488  }, // 1e-98
        pow_table_entry{  0x1.b5733cb32b111p-323, -0x1.1d8b502a64b8ep-377, 0x1.7b3b5d38057a8p-432, -0x1.7a21a749a771ep-489  }, // 1e-97
        pow_table_entry{  0x1.116805effaeaap-319, 0x1.cd88ede5810c7p-373, 0x1.76828d2181b64p-427, 0x1.f13aaf771f759p-481  }, // 1e-96
        pow_table_entry{  0x1.55c2076bf9a55p-316, 0x1.03aca57b853e5p-372, -0x1.5ee67cb0eee13p-427, -0x1.276a4ab18ad10p-482  }, // 1e-95
        pow_table_entry{  0x1.ab328946f80eap-313, 0x1.5125f3b699a38p-367, -0x1.f6d4037ba5533p-421, 0x1.1d76454424f59p-480  }, // 1e-94
        pow_table_entry{  0x1.0aff95cc5b092p-309, 0x1.d2b7b85220063p-363, -0x1.3a44822d47540p-417, 0x1.0b269eb4a9719p-472  }, // 1e-93
        pow_table_entry{  0x1.4dbf7b3f71cb7p-306, 0x1.1d96999aa01edp-362, 0x1.dca9751d9b5c1p-416, -0x1.641f733c58640p-470  }, // 1e-92
        pow_table_entry{  0x1.a12f5a0f4e3e5p-303, -0x1.4d81dfff5beccp-358, 0x1.a9e9e93281198p-412, 0x1.a16c57fa48c18p-466  }, // 1e-91
        pow_table_entry{  0x1.04bd984990e6fp-299, 0x1.7c76a00334606p-357, 0x1.464637f215fe8p-413, 0x1.38edbf1b5e3b9p-468  }, // 1e-90
        pow_table_entry{  0x1.45ecfe5bf520bp-296, -0x1.c48d76ff7fd0fp-351, -0x1.9a0a0e8459207p-408, -0x1.cf1ada23b946bp-462  }, // 1e-89
        pow_table_entry{  0x1.97683df2f268dp-293, 0x1.e52795a0501d7p-347, -0x1.a008c92256f69p-401, 0x1.abd1e6f535868p-455  }, // 1e-88
        pow_table_entry{  0x1.fd424d6faf031p-290, -0x1.431d09ef37b68p-345, 0x1.efea092a2697bp-399, -0x1.d2733e9afa2fdp-453  }, // 1e-87
        pow_table_entry{  0x1.3e497065cd61fp-286, -0x1.e4f9131ac1690p-340, -0x1.6506dd22d3f0ap-394, 0x1.2e3bfc6f91d11p-448  }, // 1e-86
        pow_table_entry{  0x1.8ddbcc7f40ba6p-283, 0x1.4391503d1c797p-338, 0x1.06ddae51dc4cfp-393, 0x1.e72bee2dd9155p-447  }, // 1e-85
        pow_table_entry{  0x1.f152bf9f10e90p-280, -0x1.35c52dd9ce342p-334, 0x1.d225467994d81p-388, -0x1.4f848b235852bp-443  }, // 1e-84
        pow_table_entry{  0x1.36d3b7c36a91ap-276, -0x1.8336795041c12p-331, 0x1.1aba605fe8384p-387, 0x1.7269484f46629p-442  }, // 1e-83
        pow_table_entry{  0x1.8488a5b445360p-273, 0x1.0dfdf42dd6e75p-327, -0x1.a7a5c1e2076e7p-382, 0x1.39e0734c62ff6p-436  }, // 1e-82
        pow_table_entry{  0x1.e5aacf2156838p-270, 0x1.517d71394ca12p-324, -0x1.18f325a894a06p-383, 0x1.0b1203ef77e80p-438  }, // 1e-81
        pow_table_entry{  0x1.2f8ac174d6123p-266, 0x1.a5dccd879fc96p-321, 0x1.f50680876a31cp-375, -0x1.d5914bd8a550fp-430  }, // 1e-80
        pow_table_entry{  0x1.7b6d71d20b96cp-263, 0x1.ea801d30f7784p-323, -0x1.b6fbead7683a9p-377, -0x1.5eb3d9d9d4a57p-432  }, // 1e-79
        pow_table_entry{  0x1.da48ce468e7c7p-260, 0x1.3290123e9aab2p-319, 0x1.db451a72bdb6dp-374, -0x1.db30682824e77p-428  }, // 1e-78
        pow_table_entry{  0x1.286d80ec190dcp-256, 0x1.85fcd05b39055p-310, 0x1.eca42cc21eda5p-364, -0x1.c2a3f904645c4p-418  }, // 1e-77
        pow_table_entry{  0x1.7288e1271f513p-253, 0x1.e77c04720746bp-307, -0x1.3065901ab2de5p-362, 0x1.9966117505196p-416  }, // 1e-76
        pow_table_entry{  0x1.cf2b1970e7258p-250, 0x1.615b058e89186p-304, -0x1.be3f7a10afcafp-358, 0x1.ff7f2ba48cbf6p-414  }, // 1e-75
        pow_table_entry{  0x1.217aefe690777p-246, 0x1.b9b1c6f22b5e7p-301, -0x1.6e7ac4a6dded5p-358, 0x1.fd7bda36bfbcep-413  }, // 1e-74
        pow_table_entry{  0x1.69d9abe034955p-243, 0x1.40f1c575b1b06p-301, -0x1.ca1975d09568ap-355, 0x1.f36b4311beb06p-412  }, // 1e-73
        pow_table_entry{  0x1.c45016d841baap-240, 0x1.1912e36d31e1cp-294, 0x1.bc3602cbb453dp-348, 0x1.c9c1184f58b97p-403  }, // 1e-72
        pow_table_entry{  0x1.1ab20e472914ap-236, 0x1.afabce243f2d2p-290, -0x1.d4bc7c815e973p-345, -0x1.e1e750ce688c2p-399  }, // 1e-71
        pow_table_entry{  0x1.615e91d8f359dp-233, 0x1.b96c1ad4ef863p-291, 0x1.b0a322f24e17dp-345, 0x1.2cf6d7efea871p-399  }, // 1e-70
        pow_table_entry{  0x1.b9b6364f30304p-230, 0x1.227c7218a2b68p-284, -0x1.dc66828a23cc4p-339, -0x1.b0f96e42835aep-393  }, // 1e-69
        pow_table_entry{  0x1.1411e1f17e1e3p-226, -0x1.4a7238b09a4dfp-280, -0x1.29c01196565fbp-335, 0x1.e2c8362cdbce6p-390  }, // 1e-68
        pow_table_entry{  0x1.59165a6ddda5bp-223, 0x1.62f139233f1e9p-277, 0x1.179fd4082810dp-333, 0x1.6de90ee04b07ep-389  }, // 1e-67
        pow_table_entry{  0x1.af5bf109550f2p-220, 0x1.775b0ed81dcc7p-275, -0x1.513c1b7ae6f58p-329, 0x1.7258d4a617727p-384  }, // 1e-66
        pow_table_entry{  0x1.0d9976a5d5297p-216, 0x1.754c74a3894fep-270, 0x1.5a74dda65f4d2p-326, 0x1.ceef09cf9d4f1p-381  }, // 1e-65
        pow_table_entry{  0x1.50ffd44f4a73dp-213, 0x1.a53f2398d747bp-268, 0x1.b112150ff7207p-323, -0x1.bd5533bc7b5d3p-378  }, // 1e-64
        pow_table_entry{  0x1.a53fc9631d10dp-210, -0x1.f8b889c079733p-264, 0x1.d569a53f4e888p-324, -0x1.6554055cd1a39p-378  }, // 1e-63
        pow_table_entry{  0x1.0747ddddf22a8p-206, -0x1.76e6ac3097d00p-261, 0x1.092b103a3c88bp-315, -0x1.66faa41ad0183p-369  }, // 1e-62
        pow_table_entry{  0x1.4919d5556eb52p-203, -0x1.d4a0573cbdc40p-258, 0x1.4b75d448cbaadp-312, 0x1.3f46b2de7be1cp-366  }, // 1e-61
        pow_table_entry{  0x1.9b604aaaca626p-200, 0x1.b63792f412cb0p-255, 0x1.9e53495afe959p-309, -0x1.70e7a069e525dp-363  }, // 1e-60
        pow_table_entry{  0x1.011c2eaabe7d8p-196, -0x1.dc3a884ee8823p-252, -0x1.fa17e44e41c51p-306, -0x1.9a431108bcde8p-361  }, // 1e-59
        pow_table_entry{  0x1.41633a556e1cep-193, -0x1.29a4953151516p-248, -0x1.789ddd61d2366p-303, 0x1.ff96155a89f4fp-357  }, // 1e-58
        pow_table_entry{  0x1.91bc08eac9a41p-190, 0x1.45f922c12d2d2p-244, 0x1.149d55a2dc9e1p-299, -0x1.c04232a769c6fp-353  }, // 1e-57
        pow_table_entry{  0x1.f62b0b257c0d2p-187, -0x1.6888948e87879p-241, -0x1.531daa7a361d4p-295, 0x1.67d6a0575de3bp-349  }, // 1e-56
        pow_table_entry{  0x1.39dae6f76d883p-183, 0x1.eaaa326eb4b43p-241, -0x1.3f28a8c61d244p-295, -0x1.f19dbc96551b4p-349  }, // 1e-55
        pow_table_entry{  0x1.8851a0b548ea4p-180, -0x1.b355681eb3c3ep-235, 0x1.ae21a5a10b725p-289, 0x1.323f5a8882b3cp-343  }, // 1e-54
        pow_table_entry{  0x1.ea6608e29b24dp-177, -0x1.10156113305a6p-231, -0x1.f32af87b58d89p-285, 0x1.3f67989551b05p-339  }, // 1e-53
        pow_table_entry{  0x1.327fc58da0f70p-173, -0x1.506ae55ff1c40p-230, -0x1.bfd6da68bc3abp-284, -0x1.c2fa0515678e5p-338  }, // 1e-52
        pow_table_entry{  0x1.7f1fb6f10934cp-170, -0x1.a4859eb7ee351p-227, 0x1.d0336efd14b6ap-281, -0x1.33b8865ac171ep-335  }, // 1e-51
        pow_table_entry{  0x1.dee7a4ad4b81fp-167, -0x1.06d38332f4e12p-223, -0x1.5ddfdaa1d30dep-277, 0x1.fd65603a38c6ap-334  }, // 1e-50
        pow_table_entry{  0x1.2b50c6ec4f313p-163, 0x1.56eef38009bcdp-217, 0x1.12aa0bad6e0bbp-272, -0x1.6c1a0a3db9c84p-326  }, // 1e-49
        pow_table_entry{  0x1.7624f8a762fd8p-160, 0x1.595560c018581p-215, -0x1.5156e2ce6ce2dp-270, -0x1.8e41199a5074ap-324  }, // 1e-48
        pow_table_entry{  0x1.d3ae36d13bbcep-157, 0x1.afaab8f01e6e1p-212, 0x1.694d91f7df91dp-269, 0x1.c5d3ffe36dc7dp-326  }, // 1e-47
        pow_table_entry{  0x1.244ce242c5561p-153, -0x1.e46a98d3d9f67p-209, 0x1.38741ecebaeedp-263, -0x1.d722dc008edb2p-317  }, // 1e-46
        pow_table_entry{  0x1.6d601ad376ab9p-150, 0x1.a27ac0f72f8c0p-206, -0x1.796ed97d96558p-260, -0x1.4ceb9300b291ep-314  }, // 1e-45
        pow_table_entry{  0x1.c8b8218854567p-147, 0x1.82c65c4d3edbcp-201, -0x1.d7ca8fdcfbeaep-257, -0x1.a02677c0df365p-311  }, // 1e-44
        pow_table_entry{  0x1.1d7314f534b61p-143, -0x1.8e44064fb8b6bp-197, 0x1.b648598578a35p-251, -0x1.010602b622e08p-305  }, // 1e-43
        pow_table_entry{  0x1.64cfda3281e39p-140, -0x1.e3aa0fc74dc8ap-195, -0x1.b84b20325267cp-249, -0x1.051e0d8eae627p-304  }, // 1e-42
        pow_table_entry{  0x1.be03d0bf225c7p-137, -0x1.72524ee484eb4p-194, -0x1.32ef41f7380d9p-249, -0x1.199643c967ec4p-303  }, // 1e-41
        pow_table_entry{  0x1.16c262777579cp-133, 0x1.631191d6259dap-187, -0x1.2ff5624ea0c22p-243, 0x1.a0042b443e18bp-300  }, // 1e-40
        pow_table_entry{  0x1.5c72fb1552d83p-130, 0x1.bbd5f64baf050p-184, 0x1.a10351476dc35p-238, 0x1.904029b0aa6cfp-292  }, // 1e-39
        pow_table_entry{  0x1.b38fb9daa78e4p-127, 0x1.2acb73de9ac65p-181, -0x1.f6bbda66b6cbdp-235, -0x1.0bafcbe32af7dp-289  }, // 1e-38
        pow_table_entry{  0x1.1039d428a8b8fp-123, -0x1.4540d794df441p-177, -0x1.746ad100647edp-232, 0x1.b16441240a4a4p-286  }, // 1e-37
        pow_table_entry{  0x1.54484932d2e72p-120, 0x1.696ef285e8eafp-174, -0x1.e8c2c2a03ecf4p-228, 0x1.1dbd516d0cdcdp-283  }, // 1e-36
        pow_table_entry{  0x1.a95a5b7f87a0fp-117, -0x1.e1aa86c4e6d2fp-174, 0x1.d0c8cb7b17cf3p-229, -0x1.ad35a37afebfap-284  }, // 1e-35
        pow_table_entry{  0x1.09d8792fb4c49p-113, 0x1.5a5ead789df78p-167, 0x1.6227d7f2ceee1p-221, 0x1.ef9df3ce99064p-275  }, // 1e-34
        pow_table_entry{  0x1.4c4e977ba1f5cp-110, -0x1.4f09a7293a8aap-164, 0x1.bab1cdef82a9ap-218, -0x1.28f51e7b81706p-273  }, // 1e-33
        pow_table_entry{  0x1.9f623d5a8a733p-107, -0x1.a2cc10f3892d4p-161, 0x1.4af20b5b1aa03p-218, -0x1.ccc998698731cp-272  }, // 1e-32
        pow_table_entry{  0x1.039d665896880p-103, -0x1.85bf8a9835bc4p-157, -0x1.e625171ce1eb8p-211, 0x1.9800802f82e04p-266  }, // 1e-31
        pow_table_entry{  0x1.4484bfeebc2a0p-100, -0x1.e72f6d3e432b6p-154, 0x1.a051a31be599ap-208, 0x1.fe00a03b63984p-263  }, // 1e-30
        pow_table_entry{  0x1.95a5efea6b347p-97, 0x1.9f04b7722c09dp-151, 0x1.0cc17c5be001ap-210, -0x1.3f9bdae1c0d39p-267  }, // 1e-29
        pow_table_entry{  0x1.fb0f6be506019p-94, 0x1.06c5e54eb70c4p-148, 0x1.0a7f8edb96c01p-202, 0x1.ce0fa5ccb9defp-261  }, // 1e-28
        pow_table_entry{  0x1.3ce9a36f23c10p-90, -0x1.b788a15d9b30bp-145, 0x1.4d1f72927c701p-199, 0x1.120c9c79ff42bp-253  }, // 1e-27
        pow_table_entry{  0x1.8c240c4aecb14p-87, -0x1.12b564da80fe7p-141, 0x1.5033a79b8dc61p-195, -0x1.a9703c6780ecap-250  }, // 1e-26
        pow_table_entry{  0x1.ef2d0f5da7dd9p-84, -0x1.5762be11213e0p-138, -0x1.5bbf6e7d8ec87p-192, -0x1.3cc4b816127c4p-251  }, // 1e-25
        pow_table_entry{  0x1.357c299a88ea7p-80, 0x1.a96249354b394p-134, -0x1.b2af4a1cf27a9p-189, 0x1.e740a19e468e5p-244  }, // 1e-24
        pow_table_entry{  0x1.82db34012b251p-77, 0x1.13badb829e079p-131, -0x1.0fad8e52178c9p-185, -0x1.e7bbcd7e89f39p-239  }, // 1e-23
        pow_table_entry{  0x1.e392010175ee6p-74, -0x1.a7566d9cba769p-128, -0x1.4e63c79a75befp-184, -0x1.86ab0378b1c1bp-238  }, // 1e-22
        pow_table_entry{  0x1.2e3b40a0e9b4fp-70, 0x1.f769fb7e0b75ep-124, 0x1.4bc068cfdd9a3p-178, -0x1.9d0ab88adbc64p-232  }, // 1e-21
        pow_table_entry{  0x1.79ca10c924223p-67, 0x1.75447a5d8e536p-121, -0x1.853df3f0abfd3p-177, -0x1.1359ab64adf4ep-235  }, // 1e-20
        pow_table_entry{  0x1.d83c94fb6d2acp-64, 0x1.a52b31e9e3d07p-119, -0x1.e68d70ecd6fc8p-174, 0x1.d4f9fd3844d1cp-229  }, // 1e-19
        pow_table_entry{  0x1.2725dd1d243acp-60, -0x1.7c628066e8ceep-114, 0x1.cfe7996bf9a23p-170, 0x1.251c3e432b031p-225  }, // 1e-18
        pow_table_entry{  0x1.70ef54646d497p-57, -0x1.db7b2080a3029p-111, -0x1.6f07a00e41fd5p-165, -0x1.2339645814785p-223  }, // 1e-17
        pow_table_entry{  0x1.cd2b297d889bcp-54, 0x1.5b4c2ebe68799p-109, -0x1.95931023a4f95p-163, 0x1.d27f08523ccd3p-217  }, // 1e-16
        pow_table_entry{  0x1.203af9ee75616p-50, -0x1.937831647f5a0p-104, -0x1.3ebdf50b238dep-158, -0x1.ae384d664cffep-212  }, // 1e-15
        pow_table_entry{  0x1.6849b86a12b9bp-47, 0x1.ea70909833de7p-107, 0x1.928db2138e9fap-163, -0x1.c660bfe03fd78p-217  }, // 1e-14
        pow_table_entry{  0x1.c25c268497682p-44, -0x1.ecd79a5a0df95p-99, 0x1.bee623d30e48fp-157, -0x1.bfc77ec27e6aep-217  }, // 1e-13
        pow_table_entry{  0x1.19799812dea11p-40, 0x1.97f27f0f6e886p-96, -0x1.ba2c0a6705c4ap-151, 0x1.5ee82350c6710p-205  }, // 1e-12
        pow_table_entry{  0x1.5fd7fe1796495p-37, 0x1.7f7bc7b4d28aap-91, -0x1.8a2dc34031cd7p-146, -0x1.25774f6c1fcb1p-204  }, // 1e-11
        pow_table_entry{  0x1.b7cdfd9d7bdbbp-34, -0x1.20a5465df8d2cp-88, 0x1.09a365f7e0dfap-142, -0x1.8b76a91a393dfp-196  }, // 1e-10
        pow_table_entry{  0x1.12e0be826d695p-30, -0x1.34674bfabb83bp-84, -0x1.59f9e04513744p-138, 0x1.1abac9f387296p-197  }, // 1e-9
        pow_table_entry{  0x1.5798ee2308c3ap-27, -0x1.03023df2d4c94p-82, -0x1.60f0b0acb0a2ap-136, 0x1.61697c7068f3bp-194  }, // 1e-8
        pow_table_entry{  0x1.ad7f29abcaf48p-24, 0x1.5e1e99483b023p-78, 0x1.23699194119a6p-132, -0x1.e463c24737ccfp-187  }, // 1e-7
        pow_table_entry{  0x1.0c6f7a0b5ed8dp-20, 0x1.b5a63f9a49c2cp-75, 0x1.b10fd7e45803dp-131, -0x1.75f2cb641700dp-186  }, // 1e-6
        pow_table_entry{  0x1.4f8b588e368f1p-17, -0x1.ee78183f91e64p-71, -0x1.bc558644523f6p-125, -0x1.fd36f7e3d1cc1p-179  }, // 1e-5
        pow_table_entry{  0x1.a36e2eb1c432dp-14, -0x1.6a161e4f765fep-68, 0x1.d495182a9930cp-122, -0x1.f212d77318fc5p-178  }, // 1e-4
        pow_table_entry{  0x1.0624dd2f1a9fcp-10, -0x1.89374bc6a7efap-66, 0x1.26e978d4fdf3bp-121, 0x1.916872b020c4ap-175  }, // 1e-3
        pow_table_entry{  0x1.47ae147ae147bp-7, -0x1.eb851eb851eb8p-63, -0x1.47ae147ae147bp-117, 0x1.eb851eb851eb8p-173  }, // 1e-2
        pow_table_entry{  0x1.999999999999ap-4, -0x1.999999999999ap-58, 0x1.999999999999ap-112, -0x1.999999999999ap-166  }, // 1e-1
        pow_table_entry{  0x1.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e0
        pow_table_entry{  0x1.4000000000000p+3, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e1
        pow_table_entry{  0x1.9000000000000p+6, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e2
        pow_table_entry{  0x1.f400000000000p+9, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e3
        pow_table_entry{  0x1.3880000000000p+13, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e4
        pow_table_entry{  0x1.86a0000000000p+16, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e5
        pow_table_entry{  0x1.e848000000000p+19, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e6
        pow_table_entry{  0x1.312d000000000p+23, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e7
        pow_table_entry{  0x1.7d78400000000p+26, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e8
        pow_table_entry{  0x1.dcd6500000000p+29, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e9
        pow_table_entry{  0x1.2a05f20000000p+33, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e10
        pow_table_entry{  0x1.74876e8000000p+36, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e11
        pow_table_entry{  0x1.d1a94a2000000p+39, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e12
        pow_table_entry{  0x1.2309ce5400000p+43, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e13
        pow_table_entry{  0x1.6bcc41e900000p+46, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e14
        pow_table_entry{  0x1.c6bf526340000p+49, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e15
        pow_table_entry{  0x1.1c37937e08000p+53, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e16
        pow_table_entry{  0x1.6345785d8a000p+56, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e17
        pow_table_entry{  0x1.bc16d674ec800p+59, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e18
        pow_table_entry{  0x1.158e460913d00p+63, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e19
        pow_table_entry{  0x1.5af1d78b58c40p+66, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e20
        pow_table_entry{  0x1.b1ae4d6e2ef50p+69, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e21
        pow_table_entry{  0x1.0f0cf064dd592p+73, 0x0.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e22
        pow_table_entry{  0x1.52d02c7e14af6p+76, 0x1.0000000000000p+23, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e23
        pow_table_entry{  0x1.a784379d99db4p+79, 0x1.0000000000000p+24, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e24
        pow_table_entry{  0x1.08b2a2c280291p+83, -0x1.b000000000000p+29, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e25
        pow_table_entry{  0x1.4adf4b7320335p+86, -0x1.1c00000000000p+32, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e26
        pow_table_entry{  0x1.9d971e4fe8402p+89, -0x1.8c00000000000p+33, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e27
        pow_table_entry{  0x1.027e72f1f1281p+93, 0x1.8440000000000p+38, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e28
        pow_table_entry{  0x1.431e0fae6d721p+96, 0x1.f2a8000000000p+42, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e29
        pow_table_entry{  0x1.93e5939a08ceap+99, -0x1.215c000000000p+44, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e30
        pow_table_entry{  0x1.f8def8808b024p+102, 0x1.4b26800000000p+48, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e31
        pow_table_entry{  0x1.3b8b5b5056e17p+106, -0x1.3107f00000000p+52, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e32
        pow_table_entry{  0x1.8a6e32246c99cp+109, 0x1.82b6140000000p+55, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e33
        pow_table_entry{  0x1.ed09bead87c03p+112, 0x1.e363990000000p+58, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e34
        pow_table_entry{  0x1.3426172c74d82p+116, 0x1.5c3c7f4000000p+61, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e35
        pow_table_entry{  0x1.812f9cf7920e3p+119, -0x1.265a307800000p+65, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e36
        pow_table_entry{  0x1.e17b84357691bp+122, 0x1.900f436a00000p+68, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e37
        pow_table_entry{  0x1.2ced32a16a1b1p+126, 0x1.e826288900000p+70, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e38
        pow_table_entry{  0x1.78287f49c4a1dp+129, 0x1.988becaad0000p+75, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e39
        pow_table_entry{  0x1.d6329f1c35ca5p+132, -0x1.0151182a7c000p+78, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e40
        pow_table_entry{  0x1.25dfa371a19e7p+136, -0x1.069578d46c000p+79, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e41
        pow_table_entry{  0x1.6f578c4e0a061p+139, -0x1.29075ae130e00p+85, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e42
        pow_table_entry{  0x1.cb2d6f618c879p+142, -0x1.cd24c665f4600p+86, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e43
        pow_table_entry{  0x1.1efc659cf7d4cp+146, -0x1.c80dbeffee2f0p+92, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e44
        pow_table_entry{  0x1.66bb7f0435c9ep+149, 0x1.c5eed14016454p+95, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e45
        pow_table_entry{  0x1.c06a5ec5433c6p+152, 0x1.bb542c80deb48p+95, 0x0.0000000000000p+0, 0x0.0000000000000p+0  }, // 1e46
        pow_table_entry{  0x1.18427b3b4a05cp+156, -0x1.babad90bdd33dp+101, 0x1.0000000000000p+47, 0x0.0000000000000p+0  }, // 1e47
        pow_table_entry{  0x1.5e531a0a1c873p+159, -0x1.14b4c7a76a406p+105, 0x1.0000000000000p+48, 0x0.0000000000000p+0  }, // 1e48
        pow_table_entry{  0x1.b5e7e08ca3a8fp+162, 0x1.a61e066ebb2f9p+108, -0x1.d800000000000p+54, 0x0.0000000000000p+0  }, // 1e49
        pow_table_entry{  0x1.11b0ec57e649ap+166, -0x1.782d3bfacb025p+112, 0x1.5900000000000p+58, 0x0.0000000000000p+0  }, // 1e50
        pow_table_entry{  0x1.561d276ddfdc0p+169, 0x1.4e3ba83411e91p+112, 0x1.7a00000000000p+58, 0x0.0000000000000p+0  }, // 1e51
        pow_table_entry{  0x1.aba4714957d30p+172, 0x1.a1ca924116636p+115, -0x1.2780000000000p+61, 0x0.0000000000000p+0  }, // 1e52
        pow_table_entry{  0x1.0b46c6cdd6e3ep+176, 0x1.051e9b68adfe2p+119, -0x1.b8b0000000000p+65, 0x0.0000000000000p+0  }, // 1e53
        pow_table_entry{  0x1.4e1878814c9cep+179, -0x1.d73337b7a4d05p+125, 0x1.f649000000000p+70, 0x0.0000000000000p+0  }, // 1e54
        pow_table_entry{  0x1.a19e96a19fc41p+182, -0x1.3400169638118p+126, 0x1.cf6d000000000p+71, 0x0.0000000000000p+0  }, // 1e55
        pow_table_entry{  0x1.05031e2503da9p+186, -0x1.b020038778c2cp+132, 0x1.2434840000000p+78, 0x0.0000000000000p+0  }, // 1e56
        pow_table_entry{  0x1.4643e5ae44d13p+189, -0x1.1c28046956f37p+135, 0x1.6d41a50000000p+81, 0x0.0000000000000p+0  }, // 1e57
        pow_table_entry{  0x1.97d4df19d6057p+192, 0x1.9ccdfa7c534fcp+138, -0x1.376df1c000000p+84, 0x0.0000000000000p+0  }, // 1e58
        pow_table_entry{  0x1.fdca16e04b86dp+195, 0x1.0401791b6823bp+141, -0x1.85496e3000000p+87, 0x0.0000000000000p+0  }, // 1e59
        pow_table_entry{  0x1.3e9e4e4c2f344p+199, 0x1.2280ebb121165p+145, -0x1.734de4de00000p+91, 0x0.0000000000000p+0  }, // 1e60
        pow_table_entry{  0x1.8e45e1df3b015p+202, 0x1.6b21269d695bep+148, -0x1.a042bc2b00000p+93, 0x0.0000000000000p+0  }, // 1e61
        pow_table_entry{  0x1.f1d75a5709c1bp+205, -0x1.3a168fbb3c4d3p+151, 0x1.f7ac94ca40000p+96, 0x0.0000000000000p+0  }, // 1e62
        pow_table_entry{  0x1.3726987666191p+209, -0x1.444e19d505b04p+155, 0x1.1d65ee7f34000p+101, 0x0.0000000000000p+0  }, // 1e63
        pow_table_entry{  0x1.84f03e93ff9f5p+212, -0x1.2ac340948e389p+157, -0x1.36812bc1fe000p+103, 0x0.0000000000000p+0  }, // 1e64
        pow_table_entry{  0x1.e62c4e38ff872p+215, 0x1.1517de8c9c729p+159, -0x1.0842ed64fb000p+105, 0x0.0000000000000p+0  }, // 1e65
        pow_table_entry{  0x1.2fdbb0e39fb47p+219, 0x1.2b4bbac5f871ep+165, 0x1.76b58ae838c80p+111, 0x0.0000000000000p+0  }, // 1e66
        pow_table_entry{  0x1.7bd29d1c87a19p+222, 0x1.d87aa5ddda398p+166, -0x1.5ce892edc8300p+111, 0x0.0000000000000p+0  }, // 1e67
        pow_table_entry{  0x1.dac74463a989fp+225, 0x1.93a653d55431fp+171, 0x1.c97ba90ad8b88p+117, 0x0.0000000000000p+0  }, // 1e68
        pow_table_entry{  0x1.28bc8abe49f64p+229, -0x1.83b80b9aab60cp+175, -0x1.6212b659388cbp+121, 0x0.0000000000000p+0  }, // 1e69
        pow_table_entry{  0x1.72ebad6ddc73dp+232, -0x1.e4a60e815638fp+178, -0x1.ba9763ef86afep+124, 0x1.0000000000000p+70  }, // 1e70
        pow_table_entry{  0x1.cfa698c95390cp+235, -0x1.5dcf9221abc73p+181, -0x1.293d3ceb685bdp+127, -0x1.8000000000000p+72  }, // 1e71
        pow_table_entry{  0x1.21c81f7dd43a7p+239, 0x1.255e44aaf4a38p+185, -0x1.ce32309909cb2p+128, 0x1.0000000000000p+72  }, // 1e72
        pow_table_entry{  0x1.6a3a275d49491p+242, 0x1.bad75756c7318p+186, -0x1.20df5e5fa61efp+132, -0x1.b000000000000p+77  }, // 1e73
        pow_table_entry{  0x1.c4c8b1349b9b5p+245, 0x1.8a634b4b1e3f7p+191, 0x1.a5ba32821c165p+137, 0x1.f900000000000p+82  }, // 1e74
        pow_table_entry{  0x1.1afd6ec0e1411p+249, 0x1.767e0f0ef2e7bp+195, -0x1.786ba06eae721p+141, 0x1.1dd0000000000p+87  }, // 1e75
        pow_table_entry{  0x1.61bcca7119916p+252, -0x1.2be26d2d505e7p+198, 0x1.29797775a5f17p+144, 0x1.9510000000000p+88  }, // 1e76
        pow_table_entry{  0x1.ba2bfd0d5ff5bp+255, 0x1.1249ef0eb713fp+200, 0x1.cf5f554c3db73p+145, 0x1.fa54000000000p+91  }, // 1e77
        pow_table_entry{  0x1.145b7e285bf99p+259, -0x1.52472a5b364e2p+202, 0x1.0cdcaa7d34941p+146, 0x1.e3a4000000000p+92  }, // 1e78
        pow_table_entry{  0x1.59725db272f7fp+262, 0x1.9649c2c37f079p+207, 0x1.95013d51c81b9p+153, 0x1.d723400000000p+97  }, // 1e79
        pow_table_entry{  0x1.afcef51f0fb5fp+265, -0x1.08f322e84da10p+204, -0x1.6f9cd67177627p+150, 0x1.9d82000000000p+95  }, // 1e80
        pow_table_entry{  0x1.0de1593369d1bp+269, 0x1.7eb4d0145d9efp+215, 0x1.ae347bf3f22acp+161, 0x1.3e02714000000p+107  }, // 1e81
        pow_table_entry{  0x1.5159af8044462p+272, 0x1.bcc40832ea0d7p+217, -0x1.cc7cca1e22951p+163, -0x1.c9f3c9c000000p+108  }, // 1e82
        pow_table_entry{  0x1.a5b01b605557bp+275, -0x1.d40af5c05b6f4p+220, 0x1.80c806b4a98b5p+165, -0x1.e385e18000000p+108  }, // 1e83
        pow_table_entry{  0x1.078e111c3556dp+279, -0x1.12436ccc1c92cp+225, -0x1.87c17de78b047p+170, -0x1.d2e33acf00000p+116  }, // 1e84
        pow_table_entry{  0x1.4971956342ac8p+282, -0x1.5b511ffc8edddp+226, 0x1.64e229e923a6bp+169, -0x1.e70260b000000p+113  }, // 1e85
        pow_table_entry{  0x1.9bcdfabc1357ap+285, -0x1.b22567fbb2954p+229, -0x1.907952e724dbfp+174, 0x1.99f3d07240000p+120  }, // 1e86
        pow_table_entry{  0x1.0160bcb58c16cp+289, 0x1.78544f8158316p+234, -0x1.3e92f4f41dc26p+180, 0x1.c01c3123b4000p+125  }, // 1e87
        pow_table_entry{  0x1.41b8ebe2ef1c7p+292, 0x1.d6696361ae3dbp+237, 0x1.c721373b6b343p+181, 0x1.8119eb6508000p+125  }, // 1e88
        pow_table_entry{  0x1.922726dbaae39p+295, 0x1.300ef0e867348p+238, -0x1.c7167af5b9fecp+184, -0x1.0f4fcce0db000p+129  }, // 1e89
        pow_table_entry{  0x1.f6b0f092959c7p+298, 0x1.2f8255a450203p+244, 0x1.71c8f99335e06p+189, 0x1.ab370ff9bb900p+134  }, // 1e90
        pow_table_entry{  0x1.3a2e965b9d81dp+302, -0x1.c24e8a794debep+248, -0x1.8e26403fe53c2p+189, 0x1.604d3f82a7400p+133  }, // 1e91
        pow_table_entry{  0x1.88ba3bf284e24p+305, -0x1.32e22d17a166ep+251, 0x1.f072817d810bap+197, 0x1.b370c11ec6a22p+143  }, // 1e92
        pow_table_entry{  0x1.eae8caef261adp+308, -0x1.7f9ab85d89c09p+254, 0x1.b23c8773853a4p+198, 0x1.02678b33c2554p+143  }, // 1e93
        pow_table_entry{  0x1.32d17ed577d0cp+312, -0x1.bf02cce9d8616p+256, -0x1.e13456af99773p+201, 0x1.43016e00b2ea9p+146  }, // 1e94
        pow_table_entry{  0x1.7f85de8ad5c4fp+315, -0x1.1761c012273cep+260, 0x1.a67e93a4802b0p+204, 0x1.c9e0e4c06fd2ap+150  }, // 1e95
        pow_table_entry{  0x1.df67562d8b363p+318, -0x1.ae9d180b58861p+264, 0x1.4203c711b406cp+210, -0x1.b874dc41ee871p+156  }, // 1e96
        pow_table_entry{  0x1.2ba095dc7701ep+322, -0x1.8d222f071753cp+268, -0x1.b6bda394ef7bdp+214, 0x1.d96decad95d72p+159  }, // 1e97
        pow_table_entry{  0x1.7688bb5394c25p+325, 0x1.f2a8a6e45ae8fp+266, -0x1.1b431e8ad6afep+211, 0x1.f92cfb1f699d5p+157  }, // 1e98
        pow_table_entry{  0x1.d42aea2879f2ep+328, 0x1.137a9684eb8d2p+274, -0x1.ad884f98b6317p+220, 0x1.8eef073ce8809p+163  }, // 1e99
        pow_table_entry{  0x1.249ad2594c37dp+332, -0x1.4f4d87b3b31f4p+276, -0x1.8ea637ee3bdcbp+219, -0x1.aaa6de7babe8cp+161  }, // 1e100
        pow_table_entry{  0x1.6dc186ef9f45cp+335, 0x1.2e6f8b2fb00c7p+280, 0x1.e0db03a16352cp+226, 0x1.1bd55ed3cad24p+171  }, // 1e101
        pow_table_entry{  0x1.c931e8ab87173p+338, 0x1.7a0b6dfb9c0f9p+283, 0x1.5911c489bc277p+229, 0x1.62cab688bd86dp+174  }, // 1e102
        pow_table_entry{  0x1.1dbf316b346e8p+342, -0x1.3b8db42be7643p+283, 0x1.7ab1ad61598a8p+229, -0x1.120a6f544c5e1p+175  }, // 1e103
        pow_table_entry{  0x1.652efdc6018a2p+345, -0x1.8a712136e13d3p+286, -0x1.26a1e7465012ep+232, -0x1.568d0b295f75ap+178  }, // 1e104
        pow_table_entry{  0x1.be7abd3781ecap+348, 0x1.f09794b3db33ap+294, -0x1.03825308bf20cp+240, 0x1.853cfb20c48adp+185  }, // 1e105
        pow_table_entry{  0x1.170cb642b133fp+352, -0x1.c9a1430f96ffcp+298, 0x1.773a306a222e2p+242, 0x1.e68c39e8f5ad8p+188  }, // 1e106
        pow_table_entry{  0x1.5ccfe3d35d80ep+355, 0x1.87ecd8590680ap+300, 0x1.d508bc84aab9bp+245, 0x1.80bd218ccc639p+189  }, // 1e107
        pow_table_entry{  0x1.b403dcc834e12p+358, -0x1.0b0bf8c85befap+304, 0x1.9292bae9755a0p+250, 0x1.de0ec69efff7cp+196  }, // 1e108
        pow_table_entry{  0x1.108269fd210cbp+362, 0x1.6462120b1a290p+306, -0x1.1912cb85a9eedp+248, -0x1.4db0f7280148dp+194  }, // 1e109
        pow_table_entry{  0x1.54a3047c694fep+365, -0x1.2142b4b90fa66p+310, -0x1.5f577e67146a9p+251, 0x1.5ee2cb0dfe650p+197  }, // 1e110
        pow_table_entry{  0x1.a9cbc59b83a3dp+368, 0x1.4b364f0c56380p+314, 0x1.f246950ff933dp+259, 0x1.a5b4dbee8beffp+205  }, // 1e111
        pow_table_entry{  0x1.0a1f5b8132466p+372, 0x1.4f01f167b5e30p+318, 0x1.376c1d29fbc06p+263, 0x1.879109751775fp+209  }, // 1e112
        pow_table_entry{  0x1.4ca732617ed80p+375, -0x1.74f648f97290fp+319, -0x1.eae36e2e153e0p+264, -0x1.68ab42da2ac8bp+208  }, // 1e113
        pow_table_entry{  0x1.9fd0fef9de8e0p+378, -0x1.d233db37cf353p+322, -0x1.967126e66a360p+265, -0x1.c2d61390b57aep+211  }, // 1e114
        pow_table_entry{  0x1.03e29f5c2b18cp+382, -0x1.23606902e1814p+326, 0x1.80fca3d7fecf2p+270, -0x1.19c5cc3a716cdp+215  }, // 1e115
        pow_table_entry{  0x1.44db473335defp+385, -0x1.6c38834399e19p+329, 0x1.e13bcccdfe82ep+273, 0x1.4fe4605b791c0p+219  }, // 1e116
        pow_table_entry{  0x1.961219000356bp+388, -0x1.71d1a90520168p+334, 0x1.cb3158002fc47p+279, 0x1.e8f75e1c95d8cp+224  }, // 1e117
        pow_table_entry{  0x1.fb969f40042c5p+391, 0x1.31b9ecb997e3ep+337, 0x1.1efed7001ddadp+283, -0x1.e732b297112c4p+229  }, // 1e118
        pow_table_entry{  0x1.3d3e2388029bbp+395, 0x1.3f1433f3feee7p+341, -0x1.3282e67fb55d1p+285, 0x1.3e01418655115p+231  }, // 1e119
        pow_table_entry{  0x1.8c8dac6a0342ap+398, 0x1.1db281e1fd541p+343, 0x1.406e2ff02ea5ep+289, -0x1.b93f370c0ad53p+235  }, // 1e120
        pow_table_entry{  0x1.efb1178484135p+401, -0x1.4d706ed2c1ab7p+347, -0x1.6f764413c5b0bp+292, -0x1.3c7826786c53ap+235  }, // 1e121
        pow_table_entry{  0x1.35ceaeb2d28c1p+405, -0x1.4199150ee42cap+349, 0x1.a561573a47192p+292, -0x1.8b96301687689p+238  }, // 1e122
        pow_table_entry{  0x1.83425a5f872f1p+408, 0x1.370052d6b1642p+353, -0x1.be28ca5ee4e41p+298, -0x1.fb9eef070a50bp+243  }, // 1e123
        pow_table_entry{  0x1.e412f0f768fadp+411, 0x1.c26033c62ede9p+357, 0x1.d24d030961e2ep+301, 0x1.c2bcaa9b998d9p+247  }, // 1e124
        pow_table_entry{  0x1.2e8bd69aa19ccp+415, 0x1.997c205bdd4b2p+361, -0x1.3723f78688b49p+307, 0x1.066d7aa84ffe2p+253  }, // 1e125
        pow_table_entry{  0x1.7a2ecc414a03fp+418, 0x1.ffdb2872d49dep+364, 0x1.ec4c2a5f54794p+308, 0x1.202365498ff6ap+254  }, // 1e126
        pow_table_entry{  0x1.d8ba7f519c84fp+421, 0x1.7fd1f28f89c56p+367, -0x1.662832c2359a2p+313, 0x1.5a0b0fa6fcfd1p+259  }, // 1e127
        pow_table_entry{  0x1.27748f9301d32p+425, -0x1.901cc86649e4ap+371, -0x1.dfd91fb961805p+317, -0x1.3dc8b1bd0f0ebp+260  }, // 1e128
        pow_table_entry{  0x1.7151b377c247ep+428, 0x1.7b80b0047445dp+369, 0x1.06130b08c3f36p+315, 0x1.cb14874eb4b6ap+261  }, // 1e129
        pow_table_entry{  0x1.cda62055b2d9ep+431, -0x1.f12cf91fd3754p+377, -0x1.6dc34191a8588p+323, 0x1.03dd9a92261e4p+268  }, // 1e130
        pow_table_entry{  0x1.2087d4358fc82p+435, 0x1.c943e44c1bd6bp+381, 0x1.1b65f704f6c8bp+327, 0x1.44d50136afa5dp+271  }, // 1e131
        pow_table_entry{  0x1.68a9c942f3ba3p+438, 0x1.dca6eaf916631p+381, -0x1.dc08b39cb8522p+326, -0x1.a7d6f9ee91c2dp+272  }, // 1e132
        pow_table_entry{  0x1.c2d43b93b0a8cp+441, -0x1.6b0bd69229011p+386, 0x1.eb3d47df06665p+331, 0x1.f719a3cae4e64p+276  }, // 1e133
        pow_table_entry{  0x1.19c4a53c4e697p+445, 0x1.8e8c4cf2532fbp+391, -0x1.e67cd98a4e000p+336, -0x1.7163fe684c3c0p+282  }, // 1e134
        pow_table_entry{  0x1.6035ce8b6203dp+448, 0x1.e45ec05dcff73p+393, -0x1.80703fb386002p+337, 0x1.92180fed05a7cp+282  }, // 1e135
        pow_table_entry{  0x1.b843422e3a84dp+451, -0x1.d144c7c55e058p+397, -0x1.782313e819e01p+342, 0x1.bed3c27d08e23p+288  }, // 1e136
        pow_table_entry{  0x1.132a095ce4930p+455, -0x1.4595f9b6b586ep+400, -0x1.d62bd8e220581p+345, 0x1.2e88b31c4b1acp+291  }, // 1e137
        pow_table_entry{  0x1.57f48bb41db7cp+458, -0x1.96fb782462e8ap+403, 0x1.b44930e55791fp+348, 0x1.e8ab7f8d7785dp+292  }, // 1e138
        pow_table_entry{  0x1.adf1aea12525bp+461, -0x1.fcba562d7ba2cp+406, -0x1.dea482e152899p+351, -0x1.9d29a08f2a98cp+295  }, // 1e139
        pow_table_entry{  0x1.0cb70d24b7379p+465, -0x1.1efa3aee36a2ep+411, 0x1.6a6c971996350p+356, 0x1.3f717ee9a1582p+301  }, // 1e140
        pow_table_entry{  0x1.4fe4d06de5057p+468, -0x1.9ae326a7112e5p+412, -0x1.d7c2190021edep+356, -0x1.c2c8856fd9475p+302  }, // 1e141
        pow_table_entry{  0x1.a3de04895e46dp+471, -0x1.8066fc14355e8p+417, 0x1.9b24d60bfd597p+363, -0x1.8337aa6cbcf99p+309  }, // 1e142
        pow_table_entry{  0x1.066ac2d5daec4p+475, -0x1.c1017632856c3p+419, 0x1.ee0b8efcafc47p+358, -0x1.6541fb0dfdc90p+298  }, // 1e143
        pow_table_entry{  0x1.4805738b51a75p+478, -0x1.18a0e9df9363ap+423, 0x1.04d31ce577b77p+368, -0x1.3a0df493ce8bfp+314  }, // 1e144
        pow_table_entry{  0x1.9a06d06e26112p+481, 0x1.426db7510f86fp+425, 0x1.4607e41ed5a54p+371, 0x1.776e8e473dd12p+317  }, // 1e145
        pow_table_entry{  0x1.00444244d7cabp+485, 0x1.326124a4aa6d1p+431, 0x1.92f13ba4d161dp+377, 0x1.75528c7643516p+322  }, // 1e146
        pow_table_entry{  0x1.405552d60dbd6p+488, 0x1.fbe5b73754217p+432, -0x1.0a4eae3f48b71p+375, 0x1.2a72f93d425afp+321  }, // 1e147
        pow_table_entry{  0x1.906aa78b912ccp+491, -0x1.614836beb5b59p+437, 0x1.6b31da630e51bp+382, 0x1.8ea1f6f1925e3p+327  }, // 1e148
        pow_table_entry{  0x1.f485516e7577fp+494, -0x1.b99a446e6322fp+440, -0x1.d00d782170cf0p+382, -0x1.b6b16a412147bp+325  }, // 1e149
        pow_table_entry{  0x1.38d352e5096afp+498, 0x1.affe54ec0828ap+442, 0x1.b77de53ac65fap+388, 0x1.f76e88ecba59ap+334  }, // 1e150
        pow_table_entry{  0x1.8708279e4bc5bp+501, -0x1.e40215d8f5cd3p+445, 0x1.2aeaf44bbfbc9p+388, -0x1.56ba9b02e2000p+332  }, // 1e151
        pow_table_entry{  0x1.e8ca3185deb72p+504, -0x1.9740a6d3ccd02p+450, 0x1.2eb4b62bd5f57p+394, 0x1.929cb5f1e32c0p+340  }, // 1e152
        pow_table_entry{  0x1.317e5ef3ab327p+508, 0x1.7797bb9ffdeccp+446, -0x1.6787124d234b1p+391, 0x1.d0f8db96fdbffp+337  }, // 1e153
        pow_table_entry{  0x1.7dddf6b095ff1p+511, -0x1.fc5504aaf0053p+456, -0x1.382d1adc0d83cp+397, 0x1.a8a6e24f97a60p+343  }, // 1e154
        pow_table_entry{  0x1.dd55745cbb7edp+514, -0x1.eda91756b019fp+457, -0x1.8638619310e4ap+400, -0x1.ed2f651c82708p+346  }, // 1e155
        pow_table_entry{  0x1.2a5568b9f52f4p+518, 0x1.65bb28b4e8f7ep+462, 0x1.30c1cc3041571p+408, 0x1.72f098338b9e7p+352  }, // 1e156
        pow_table_entry{  0x1.74eac2e8727b1p+521, 0x1.bf29f2e22335ep+465, -0x1.061b81875ca65p+410, -0x1.1829a0dfc8bd0p+356  }, // 1e157
        pow_table_entry{  0x1.d22573a28f19dp+524, 0x1.8bbd1be6ab00dp+470, 0x1.570bb3c2d9860p+416, 0x1.6872fdba1144fp+361  }, // 1e158
        pow_table_entry{  0x1.2357684599702p+528, 0x1.775631702ae08p+474, 0x1.56675059c7f3cp+420, 0x1.c28fbd2895963p+364  }, // 1e159
        pow_table_entry{  0x1.6c2d4256ffcc3p+531, -0x1.56a2119e533adp+474, 0x1.60092381cf859p+420, 0x1.999d6395d7ddcp+364  }, // 1e160
        pow_table_entry{  0x1.c73892ecbfbf4p+534, -0x1.358952c0bd013p+480, 0x1.7016d8c486cdfp+422, -0x1.fffb4384b22adp+367  }, // 1e161
        pow_table_entry{  0x1.1c835bd3f7d78p+538, 0x1.3e8a2c4789df4p+484, 0x1.1cc1c8ef5a881p+429, 0x1.9c002f5cd10a5p+375  }, // 1e162
        pow_table_entry{  0x1.63a432c8f5cd6p+541, 0x1.8e2cb7596c571p+487, 0x1.63f23b2b312a2p+432, -0x1.f9ff8997f5663p+377  }, // 1e163
        pow_table_entry{  0x1.bc8d3f7b3340cp+544, -0x1.c9035a0712651p+485, -0x1.889b050145ae8p+428, 0x1.e02500835011bp+374  }, // 1e164
        pow_table_entry{  0x1.15d847ad00087p+548, 0x1.f712ef3ddca40p+494, 0x1.ab0a9f1cdf347p+440, 0x1.7a582e40a4241p+385  }, // 1e165
        pow_table_entry{  0x1.5b4e5998400a9p+551, 0x1.74d7ab0d53cd1p+497, -0x1.ea32b91be8fe7p+443, -0x1.388e317996972p+385  }, // 1e166
        pow_table_entry{  0x1.b221effe500d4p+554, -0x1.2df26a2f573fbp+500, -0x1.64bf6762e33e1p+446, 0x1.cf29c84500786p+391  }, // 1e167
        pow_table_entry{  0x1.0f5535fef2084p+558, 0x1.43487da269783p+504, -0x1.7bde8277381b2p+448, 0x1.0bd0e9590259fp+392  }, // 1e168
        pow_table_entry{  0x1.532a837eae8a5p+561, 0x1.941a9d0b03d64p+507, -0x1.76b588c541888p+453, 0x1.94ec523af42f0p+399  }, // 1e169
        pow_table_entry{  0x1.a7f5245e5a2cfp+564, -0x1.06debbb23b343p+510, -0x1.d462eaf691eaap+456, 0x1.fa2766c9b13adp+402  }, // 1e170
        pow_table_entry{  0x1.08f936baf85c1p+568, 0x1.b769956135fecp+513, -0x1.497ba5b436654p+459, 0x1.e2c501f07625fp+403  }, // 1e171
        pow_table_entry{  0x1.4b378469b6732p+571, -0x1.ed5e02a33e40dp+517, 0x1.3212b86f5e00cp+463, -0x1.b49137b26d8a1p+409  }, // 1e172
        pow_table_entry{  0x1.9e056584240fep+574, -0x1.a2d60d3037440p+518, 0x1.fa5d9a2cd603ap+464, -0x1.0dac2cf84764cp+409  }, // 1e173
        pow_table_entry{  0x1.02c35f729689fp+578, -0x1.4171720f88a2ap+524, 0x1.3c7a805c05c24p+468, 0x1.577463e4d3610p+413  }, // 1e174
        pow_table_entry{  0x1.4374374f3c2c6p+581, 0x1.6e32316c9534cp+527, -0x1.9d19b7e33e335p+473, 0x1.35aa2f9bc1073p+419  }, // 1e175
        pow_table_entry{  0x1.945145230b378p+584, -0x1.b20a11c22bf0cp+527, -0x1.1809770370078p+470, 0x1.8a5dc158a4798p+415  }, // 1e176
        pow_table_entry{  0x1.f965966bce056p+587, -0x1.0f464b195b768p+531, 0x1.ea1f42b3bb3f7p+477, -0x1.7098567289934p+423  }, // 1e177
        pow_table_entry{  0x1.3bdf7e0360c36p+591, -0x1.2a62fbbbf64a8p+537, -0x1.66d63b27d57c3p+482, 0x1.334193f0d407fp+426  }, // 1e178
        pow_table_entry{  0x1.8ad75d8438f43p+594, 0x1.16088aaa1845cp+539, -0x1.c08bc9f1cadb4p+485, 0x1.60047e3b42428p+431  }, // 1e179
        pow_table_entry{  0x1.ed8d34e547314p+597, -0x1.48eaa556c351bp+541, -0x1.8575e371ec905p+485, 0x1.c02cee509698dp+431  }, // 1e180
        pow_table_entry{  0x1.3478410f4c7ecp+601, 0x1.cc9b562a717b4p+547, -0x1.de6d35c4e67b4p+492, -0x1.6cfc7d61b43c1p+438  }, // 1e181
        pow_table_entry{  0x1.819651531f9e8p+604, -0x1.c03dd44af225fp+550, -0x1.2b04419b100d1p+496, 0x1.1be231a2ef5a7p+442  }, // 1e182
        pow_table_entry{  0x1.e1fbe5a7e7861p+607, 0x1.cfb2b6a251509p+553, -0x1.d715480750414p+497, 0x1.8b6af82eacc45p+443  }, // 1e183
        pow_table_entry{  0x1.2d3d6f88f0b3dp+611, -0x1.78c1376a34b6ap+555, 0x1.b32565f6dbae7p+500, 0x1.ee45b63a57f56p+446  }, // 1e184
        pow_table_entry{  0x1.788ccb6b2ce0cp+614, 0x1.14873d5d9f0dep+559, -0x1.e011408b6d65fp+503, 0x1.69d723c8edf2cp+449  }, // 1e185
        pow_table_entry{  0x1.d6affe45f818fp+617, 0x1.59a90cb506d15p+562, 0x1.69fa9bd46dd02p+508, 0x1.b1133b2eca5bep+454  }, // 1e186
        pow_table_entry{  0x1.262dfeebbb0f9p+621, 0x1.ec04d3f892217p+567, -0x1.4ee1af4d9daefp+513, -0x1.f153fb02c1869p+458  }, // 1e187
        pow_table_entry{  0x1.6fb97ea6a9d38p+624, -0x1.31f3ee1292ac7p+569, -0x1.453436420a356p+515, -0x1.b6a3e70dc7a0fp+459  }, // 1e188
        pow_table_entry{  0x1.cba7de5054486p+627, -0x1.7e70e99737579p+572, -0x1.2d0287a519857p+517, -0x1.122670689cc4ap+463  }, // 1e189
        pow_table_entry{  0x1.1f48eaf234ad4p+631, -0x1.778348ff414b6p+577, 0x1.21ef359c68065p+522, -0x1.15ac0320b0fd7p+468  }, // 1e190
        pow_table_entry{  0x1.671b25aec1d89p+634, -0x1.d5641b3f119e3p+580, -0x1.4aca7e7e3efc1p+526, -0x1.6c5c0fa374f33p+469  }, // 1e191
        pow_table_entry{  0x1.c0e1ef1a724ebp+637, -0x1.4abd220ed605cp+583, -0x1.3afa3c3b9d763p+528, 0x1.8e233b1ceb740p+474  }, // 1e192
        pow_table_entry{  0x1.188d357087713p+641, -0x1.4eb6354945c3ap+587, 0x1.9d91cd2d5ecb1p+533, 0x1.78d604f213288p+478  }, // 1e193
        pow_table_entry{  0x1.5eb082cca94d7p+644, 0x1.5d9c3d6468cb8p+590, 0x1.3d901e2d9f75fp+530, -0x1.1e8f3a2d01abfp+476  }, // 1e194
        pow_table_entry{  0x1.b65ca37fd3a0dp+647, 0x1.6a06997b05fccp+592, 0x1.8cf425b907536p+533, 0x1.99ccf747bde91p+479  }, // 1e195
        pow_table_entry{  0x1.11f9e62fe4448p+651, 0x1.e2441fece3be0p+596, -0x1.f83f3b4362db6p+542, 0x1.000403519ad63p+486  }, // 1e196
        pow_table_entry{  0x1.56785fbbdd55ap+654, 0x1.2d6a93f40e56cp+600, -0x1.3b27850a1dc92p+546, 0x1.2800a084c0318p+492  }, // 1e197
        pow_table_entry{  0x1.ac1677aad4ab1p+657, -0x1.0e758e1ddc273p+602, 0x1.d83a66cd6b127p+547, 0x1.c8032297c0f76p+493  }, // 1e198
        pow_table_entry{  0x1.0b8e0acac4eafp+661, -0x1.d484bc6954cc4p+607, 0x1.139240203175cp+552, 0x1.4e80facf6c4d5p+498  }, // 1e199
        pow_table_entry{  0x1.4e718d7d7625ap+664, 0x1.6cb428f8ac016p+609, 0x1.5876d0283dd33p+555, 0x1.a22139834760ap+501  }, // 1e200
        pow_table_entry{  0x1.a20df0dcd3af1p+667, -0x1.1c0f6664947f2p+613, -0x1.45adef36cadffp+556, 0x1.5530fc832718dp+499  }, // 1e201
        pow_table_entry{  0x1.0548b68a044d6p+671, 0x1.ce76600123309p+617, -0x1.197196b047d98p+563, 0x1.4d53e9dd1f870p+507  }, // 1e202
        pow_table_entry{  0x1.469ae42c8560cp+674, 0x1.084fe005aff2cp+618, -0x1.7f37f171673f8p+564, 0x1.a0a8e4546768bp+510  }, // 1e203
        pow_table_entry{  0x1.98419d37a6b8fp+677, 0x1.4a63d8071bef7p+621, -0x1.df05edcdc10f5p+567, -0x1.f72ce2967ebd2p+513  }, // 1e204
        pow_table_entry{  0x1.fe52048590673p+680, -0x1.318198fb8e8a6p+625, 0x1.52712d7d9d59ap+569, 0x1.160fc987c3274p+515  }, // 1e205
        pow_table_entry{  0x1.3ef342d37a408p+684, -0x1.bef0ff9d39168p+629, 0x1.34e1af1ba0960p+575, 0x1.adc9ddf4d9f88p+519  }, // 1e206
        pow_table_entry{  0x1.8eb0138858d0ap+687, -0x1.17569fc243ae1p+633, 0x1.821a1ae288bb8p+578, 0x1.0c9e2ab9083b5p+523  }, // 1e207
        pow_table_entry{  0x1.f25c186a6f04cp+690, 0x1.45a7709a56ccep+635, -0x1.d5f5e64d5159dp+577, -0x1.81d254c5adaecp+523  }, // 1e208
        pow_table_entry{  0x1.37798f4285630p+694, -0x1.9a3baccfc4e00p+640, 0x1.76d232807d694p+586, -0x1.2e246e9f7191ap+530  }, // 1e209
        pow_table_entry{  0x1.8557f31326bbbp+697, 0x1.ff3567fc49e80p+643, 0x1.d486bf209cc39p+589, -0x1.79ad8a474df61p+533  }, // 1e210
        pow_table_entry{  0x1.e6adefd7f06aap+700, 0x1.7f02c1fb5c621p+646, -0x1.b65791173c0b9p+592, 0x1.13f389936f463p+537  }, // 1e211
        pow_table_entry{  0x1.302cb5e6f642ap+704, 0x1.ef61b93d19bd4p+650, 0x1.6e0945517a78cp+596, 0x1.d63c1afe12c5fp+542  }, // 1e212
        pow_table_entry{  0x1.7c37e360b3d35p+707, 0x1.ace89e3180b26p+651, -0x1.b3a34ad137483p+596, -0x1.a1a6f2134444ap+542  }, // 1e213
        pow_table_entry{  0x1.db45dc38e0c82p+710, 0x1.8608b16f7837cp+656, -0x1.8823076161469p+601, -0x1.0a10ae981555cp+545  }, // 1e214
        pow_table_entry{  0x1.290ba9a38c7d1p+714, 0x1.f3c56ee5ab22dp+660, 0x1.85750db19199fp+606, 0x1.566d64b83caaap+551  }, // 1e215
        pow_table_entry{  0x1.734e940c6f9c6p+717, -0x1.1e926ac1d428fp+662, 0x1.cda4a23bec00ep+608, -0x1.4fdd0866d0ab0p+552  }, // 1e216
        pow_table_entry{  0x1.d022390f8b837p+720, 0x1.4ce47d46db667p+666, -0x1.7de46a6a31fddp+610, -0x1.a3d44a8084d5cp+555  }, // 1e217
        pow_table_entry{  0x1.221563a9b7323p+724, -0x1.aff131b3b6e00p+670, 0x1.44544f5f68305p+616, 0x1.bf336a2df59f5p+562  }, // 1e218
        pow_table_entry{  0x1.6a9abc9424febp+727, 0x1.c82503beb6d01p+672, -0x1.aa5a7322f70e5p+617, 0x1.780225cb98390p+562  }, // 1e219
        pow_table_entry{  0x1.c5416bb92e3e6p+730, 0x1.d172257324208p+672, -0x1.4f10febb4d1e0p+616, -0x1.4fea860c0dc5dp+562  }, // 1e220
        pow_table_entry{  0x1.1b48e353bce70p+734, -0x1.dba31513012d7p+679, -0x1.868b54f9a8819p+625, -0x1.868f949e3c44ep+571  }, // 1e221
        pow_table_entry{  0x1.621b1c28ac20cp+737, -0x1.2945ed2be0bc7p+683, 0x1.8be8eae3f6af0p+629, 0x1.17cc863a34a9fp+574  }, // 1e222
        pow_table_entry{  0x1.baa1e332d728fp+740, -0x1.73976876d8eb8p+686, -0x1.111cda630ba54p+632, 0x1.5dbfa7c8c1d46p+577  }, // 1e223
        pow_table_entry{  0x1.14a52dffc6799p+744, 0x1.2f82bd6b70d9ap+689, -0x1.556410fbce8e9p+635, 0x1.b52f91baf2498p+580  }, // 1e224
        pow_table_entry{  0x1.59ce797fb817fp+747, 0x1.bdb1b66326880p+693, 0x1.550bab14f7374p+636, 0x1.13dbb14d76df0p+580  }, // 1e225
        pow_table_entry{  0x1.b04217dfa61dfp+750, 0x1.2d1e23fbf02a0p+696, 0x1.aa4e95da35051p+639, 0x1.58d29da0d496cp+583  }, // 1e226
        pow_table_entry{  0x1.0e294eebc7d2cp+754, -0x1.c3cd298289e5cp+700, 0x1.0a711da861233p+643, -0x1.4a1f175edec87p+589  }, // 1e227
        pow_table_entry{  0x1.51b3a2a6b9c76p+757, 0x1.cb3f8c1cd3a0dp+703, 0x1.4d0d6512796bfp+646, 0x1.635922c969857p+592  }, // 1e228
        pow_table_entry{  0x1.a6208b5068394p+760, 0x1.f07b792044482p+703, 0x1.a050be5717c6fp+649, 0x1.785ed6f787cdap+594  }, // 1e229
        pow_table_entry{  0x1.07d457124123dp+764, -0x1.d9365a897aaa6p+710, 0x1.810c9dbd9bb71p+655, 0x1.7d6768cb569c1p+601  }, // 1e230
        pow_table_entry{  0x1.49c96cd6d16ccp+767, -0x1.4f83f12bd954fp+713, -0x1.0f581d697ead9p+659, -0x1.233ebd01d3bcfp+604  }, // 1e231
        pow_table_entry{  0x1.9c3bc80c85c7fp+770, -0x1.a364ed76cfaa3p+716, -0x1.4cb8930f7963ep+660, 0x1.27e3277b6ea7bp+606  }, // 1e232
        pow_table_entry{  0x1.01a55d07d39cfp+774, 0x1.e783ae56f8d68p+718, 0x1.300ca41654219p+664, 0x1.b8edf8ad2528dp+610  }, // 1e233
        pow_table_entry{  0x1.420eb449c8843p+777, -0x1.9e9b661348f3ep+721, 0x1.7c0fcd1be92a0p+667, -0x1.b1ad124f231a0p+612  }, // 1e234
        pow_table_entry{  0x1.9292615c3aa54p+780, -0x1.81908fe606cc3p+726, -0x1.093b0fe74722ep+672, -0x1.0f0c2b7175f04p+616  }, // 1e235
        pow_table_entry{  0x1.f736f9b3494e9p+783, -0x1.e1f4b3df887f4p+729, -0x1.2e274f8463ae6p+673, -0x1.52cf364dd36c5p+619  }, // 1e236
        pow_table_entry{  0x1.3a825c100dd11p+787, 0x1.52c70f944ab07p+733, 0x1.d0c9db93506ccp+679, 0x1.61f3f07adee29p+620  }, // 1e237
        pow_table_entry{  0x1.8922f31411456p+790, -0x1.58872c86a2a37p+736, 0x1.44fc52782487fp+682, 0x1.ba70ec99969b3p+623  }, // 1e238
        pow_table_entry{  0x1.eb6bafd91596bp+793, 0x1.455c215ed2cefp+737, -0x1.a71263a749585p+683, 0x1.148693dffe210p+627  }, // 1e239
        pow_table_entry{  0x1.33234de7ad7e3p+797, -0x1.34a66b24bc3ebp+741, 0x1.de5206ddc8a34p+685, -0x1.532be394012b6p+631  }, // 1e240
        pow_table_entry{  0x1.7fec216198ddcp+800, -0x1.6074017b7ad39p+746, -0x1.9aa19776ac534p+692, 0x1.2c0491c37f44ep+635  }, // 1e241
        pow_table_entry{  0x1.dfe729b9ff153p+803, -0x1.b89101da59888p+749, 0x1.fd6c0557512fep+694, 0x1.7705b6345f162p+638  }, // 1e242
        pow_table_entry{  0x1.2bf07a143f6d4p+807, -0x1.935aa12877f55p+753, 0x1.3e63835692bdfp+698, -0x1.8ace370fa2491p+643  }, // 1e243
        pow_table_entry{  0x1.76ec98994f489p+810, -0x1.f831497295f2ap+756, -0x1.c80e6f4f224a6p+699, 0x1.27e3b2c7524a3p+642  }, // 1e244
        pow_table_entry{  0x1.d4a7bebfa31abp+813, -0x1.763d9bcf3b6f5p+759, 0x1.b8bdbe9ba2a46p+705, 0x1.171dc9f7926ddp+649  }, // 1e245
        pow_table_entry{  0x1.24e8d737c5f0bp+817, -0x1.69e6816185259p+763, 0x1.26ed2e428b4d8p+708, -0x1.a8c6b0e2a23dbp+654  }, // 1e246
        pow_table_entry{  0x1.6e230d05b76cdp+820, 0x1.3b9fde4619911p+766, -0x1.1eaf0c59a3be5p+710, -0x1.2f85d1b4acd1dp+653  }, // 1e247
        pow_table_entry{  0x1.c9abd04725481p+823, -0x1.75782a28600abp+769, 0x1.4cd29847f9a91p+714, -0x1.2f6ce8c43b00cp+659  }, // 1e248
        pow_table_entry{  0x1.1e0b622c774d0p+827, 0x1.9694e5a6c3f95p+773, 0x1.d0039f2cfc09bp+718, -0x1.ded208bd52704p+664  }, // 1e249
        pow_table_entry{  0x1.658e3ab795204p+830, 0x1.fc3a1f1074f7bp+776, -0x1.ddfdbc83e279fp+722, -0x1.ab43457653862p+668  }, // 1e250
        pow_table_entry{  0x1.bef1c9657a686p+833, -0x1.84b7592b6dca7p+779, 0x1.5505a8b649cf1p+724, 0x1.d3d7d2582f30ap+670  }, // 1e251
        pow_table_entry{  0x1.17571ddf6c814p+837, -0x1.f2f297bb249e8p+783, -0x1.156e3b4708ef5p+729, 0x1.d23371bb8ebf3p+675  }, // 1e252
        pow_table_entry{  0x1.5d2ce55747a18p+840, 0x1.9050c2561239ep+786, -0x1.5ac9ca18cb2b2p+732, 0x1.46c04e2a726f0p+678  }, // 1e253
        pow_table_entry{  0x1.b4781ead1989ep+843, 0x1.f464f2eb96c85p+789, 0x1.3a0f0d8408288p+733, -0x1.9e3e792bc3d51p+679  }, // 1e254
        pow_table_entry{  0x1.10cb132c2ff63p+847, 0x1.c5f8be99f1e99p+790, 0x1.8892d0e50a329p+736, 0x1.fa31e8894b35bp+682  }, // 1e255
        pow_table_entry{  0x1.54fdd7f73bf3cp+850, -0x1.7222446fe4670p+795, -0x1.15487ae1b340cp+739, -0x1.0e833aa8c3f9cp+684  }, // 1e256
        pow_table_entry{  0x1.aa3d4df50af0bp+853, -0x1.ceaad58bdd80cp+798, -0x1.5a9a999a2010fp+742, -0x1.52240952f4f83p+687  }, // 1e257
        pow_table_entry{  0x1.0a6650b926d67p+857, -0x1.109562bbb5384p+803, 0x1.c9d7d7ffeafd6p+748, -0x1.7a6ad0ba7b236p+694  }, // 1e258
        pow_table_entry{  0x1.4cffe4e7708c0p+860, 0x1.ab4544955d79bp+806, 0x1.1e26e6fff2de6p+752, -0x1.ec82c2748cf62p+698  }, // 1e259
        pow_table_entry{  0x1.a03fde214caf1p+863, -0x1.e9e96a454b27ep+809, 0x1.96c282ffbe57cp+753, -0x1.9e8dcc46c0ce9p+699  }, // 1e260
        pow_table_entry{  0x1.0427ead4cfed6p+867, 0x1.4dce1d94b1071p+813, 0x1.3f8e6477f5bdbp+759, 0x1.3f39d814f1dfcp+705  }, // 1e261
        pow_table_entry{  0x1.4531e58a03e8cp+870, -0x1.7af96c188adc9p+814, -0x1.c23809a8334b7p+760, -0x1.c3dec79746a16p+706  }, // 1e262
        pow_table_entry{  0x1.967e5eec84e2fp+873, -0x1.d9b7c71ead93cp+817, 0x1.9a73e7db7fc35p+762, 0x1.96530d05cf6c8p+708  }, // 1e263
        pow_table_entry{  0x1.fc1df6a7a61bbp+876, -0x1.94096e39963e3p+822, 0x1.40221c3a4bf68p+768, 0x1.5f7cfa08e868fp+714  }, // 1e264
        pow_table_entry{  0x1.3d92ba28c7d15p+880, -0x1.7c85e4e3fde6ep+826, 0x1.481551a46f7a1p+772, 0x1.b75c388b22833p+717  }, // 1e265
        pow_table_entry{  0x1.8cf768b2f9c5ap+883, -0x1.b74ebc39fac12p+828, -0x1.979567c9d29dap+773, 0x1.299a356f591ffp+717  }, // 1e266
        pow_table_entry{  0x1.f03542dfb8370p+886, 0x1.dadd94b7868e9p+831, 0x1.01429f21dc5d8p+777, -0x1.a2ffcf4d34260p+722  }, // 1e267
        pow_table_entry{  0x1.362149cbd3226p+890, 0x1.28ca7cf2b4192p+835, -0x1.be6cb915ac8b2p+780, -0x1.05dfe1904097cp+726  }, // 1e268
        pow_table_entry{  0x1.83a99c3ec7eb0p+893, -0x1.468171e84f705p+839, 0x1.d1f818a4e8521p+783, 0x1.71504c175e849p+728  }, // 1e269
        pow_table_entry{  0x1.e494034e79e5cp+896, -0x1.9821ce62634c6p+842, -0x1.b989e131dd997p+786, 0x1.e6d22f8e9b12ep+732  }, // 1e270
        pow_table_entry{  0x1.2edc82110c2f9p+900, 0x1.00eadf0281f04p+846, 0x1.7604e9a06ac01p+791, -0x1.3ef2891b7c50dp+734  }, // 1e271
        pow_table_entry{  0x1.7a93a2954f3b8p+903, -0x1.beda693cdd93bp+849, 0x1.d386240885701p+794, 0x1.9c5435276926cp+739  }, // 1e272
        pow_table_entry{  0x1.d9388b3aa30a5p+906, 0x1.d16efc73eb077p+852, -0x1.dbcc297aac99fp+798, -0x1.fe4b5ec75e47dp+743  }, // 1e273
        pow_table_entry{  0x1.27c35704a5e67p+910, 0x1.a2e55dc872e4ap+856, 0x1.5a81984d507f2p+800, -0x1.f778d9e4d766fp+744  }, // 1e274
        pow_table_entry{  0x1.71b42cc5cf601p+913, 0x1.0b9eb53a8f9ddp+859, -0x1.93b78067d6d84p+805, -0x1.a7557105e0d41p+751  }, // 1e275
        pow_table_entry{  0x1.ce2137f743382p+916, -0x1.b1799d76cc7acp+862, -0x1.f14ac103991cbp+807, -0x1.12acd4759090ep+750  }, // 1e276
        pow_table_entry{  0x1.20d4c2fa8a031p+920, -0x1.dd804d47f9975p+861, 0x1.2628ebb809c23p+806, 0x1.514fecda1695ep+752  }, // 1e277
        pow_table_entry{  0x1.6909f3b92c83dp+923, 0x1.dab1f9f660803p+868, -0x1.0904cd959f3cdp+813, -0x1.eb4b82fdec789p+758  }, // 1e278
        pow_table_entry{  0x1.c34c70a777a4dp+926, -0x1.d750c3c603afep+872, -0x1.a5a3007d83860p+817, -0x1.198798ef59e5bp+763  }, // 1e279
        pow_table_entry{  0x1.1a0fc668aac70p+930, -0x1.4d24f4b7849bep+875, 0x1.f0f43f631b988p+820, -0x1.5fe97f2b305f2p+766  }, // 1e280
        pow_table_entry{  0x1.6093b802d578cp+933, -0x1.a06e31e565c2dp+878, -0x1.92ceb0c41d816p+823, -0x1.b7e3def5fc76ep+769  }, // 1e281
        pow_table_entry{  0x1.b8b8a6038ad6fp+936, -0x1.0444df2f5f99cp+882, -0x1.fbc12e7a9270ep+827, -0x1.2ee6b59bdca4dp+769  }, // 1e282
        pow_table_entry{  0x1.137367c236c65p+940, 0x1.baa9e904c87fdp+885, -0x1.3d58bd0c9b869p+831, 0x1.e855f9cfd2c32p+776  }, // 1e283
        pow_table_entry{  0x1.585041b2c477fp+943, -0x1.eb55ce5d02b02p+889, -0x1.195dd89f84d06p+833, 0x1.89ade10f1dcfap+777  }, // 1e284
        pow_table_entry{  0x1.ae64521f7595ep+946, 0x1.33a97c177947bp+891, -0x1.5fb54ec766047p+836, -0x1.84f9a9ab46af2p+782  }, // 1e285
        pow_table_entry{  0x1.0cfeb353a97dbp+950, -0x1.3fb6127154333p+895, -0x1.dbd1513c9fc2dp+840, 0x1.8ce3f5f4f3d29p+786  }, // 1e286
        pow_table_entry{  0x1.503e602893dd2p+953, -0x1.c7d1cb86d4a00p+899, -0x1.4b16962f1ecdfp+841, -0x1.fc6191b9e719ep+784  }, // 1e287
        pow_table_entry{  0x1.a44df832b8d46p+956, -0x1.ce31f3444e400p+899, -0x1.9ddc3bbae6817p+844, 0x1.61218275e7c7fp+789  }, // 1e288
        pow_table_entry{  0x1.06b0bb1fb384cp+960, -0x1.241be701561d0p+906, -0x1.02a9a554d010ep+848, -0x1.11a5873b27918p+794  }, // 1e289
        pow_table_entry{  0x1.485ce9e7a065fp+963, -0x1.6d22e0c1aba44p+909, -0x1.43540eaa04152p+851, 0x1.53e22dec1d143p+796  }, // 1e290
        pow_table_entry{  0x1.9a742461887f6p+966, 0x1.3794670de972bp+912, -0x1.94291254851a6p+854, -0x1.2b92a34c6dd36p+800  }, // 1e291
        pow_table_entry{  0x1.008896bcf54fap+970, -0x1.ea19fcba70c29p+913, -0x1.f93356e9a6610p+857, 0x1.131167c0ed6f8p+802  }, // 1e292
        pow_table_entry{  0x1.40aabc6c32a38p+973, 0x1.b36bf082de61ap+919, -0x1.b3bc0165207fdp+865, 0x1.855f5706c4a33p+811  }, // 1e293
        pow_table_entry{  0x1.90d56b873f4c7p+976, -0x1.dfb9135c6a060p+922, -0x1.05580df344fdep+865, -0x1.948d3378a3407p+810  }, // 1e294
        pow_table_entry{  0x1.f50ac6690f1f8p+979, 0x1.50b14f98f6f10p+924, -0x1.46ae1170163d6p+868, 0x1.0327bfd499f7cp+814  }, // 1e295
        pow_table_entry{  0x1.3926bc01a973bp+983, 0x1.a4dda37f34ad4p+927, -0x1.985995cc1bccbp+871, -0x1.781ca06c7f14bp+816  }, // 1e296
        pow_table_entry{  0x1.87706b0213d0ap+986, -0x1.f1eaf3a0fe277p+930, -0x1.fe6ffb3f22bfep+874, 0x1.4ee1bbc309314p+816  }, // 1e297
        pow_table_entry{  0x1.e94c85c298c4cp+989, 0x1.646693ddb093bp+935, -0x1.cfc17f41dd6e0p+880, 0x1.4345345567970p+826  }, // 1e298
        pow_table_entry{  0x1.31cfd3999f7b0p+993, -0x1.213fe39571a3bp+939, -0x1.10ec77c495326p+885, 0x1.9416816ac17ccp+829  }, // 1e299
        pow_table_entry{  0x1.7e43c8800759cp+996, -0x1.698fdc7ace0cap+942, -0x1.549e56d6e9fbep+886, 0x1.f91c21c571dbfp+832  }, // 1e300
        pow_table_entry{  0x1.ddd4baa009303p+999, -0x1.c3f3d399818fdp+945, 0x1.958e84dcd6e15p+891, -0x1.c44e6ae498d69p+836  }, // 1e301
        pow_table_entry{  0x1.2aa4f4a405be2p+1003, -0x1.9a78643ff0f9ep+949, 0x1.f5e44c2819334p+893, -0x1.ab102cedf8619p+836  }, // 1e302
        pow_table_entry{  0x1.754e31cd072dap+1006, -0x1.167d4fed38559p+944, 0x1.aeaf990fc0078p+889, -0x1.5d438297679f9p+835  }, // 1e303
        pow_table_entry{  0x1.d2a1be4048f90p+1009, 0x1.fea3e35c17799p+955, 0x1.440d2dbfa9d80p+901, 0x1.2b25b5ce615f4p+847  }, // 1e304
        pow_table_entry{  0x1.23a516e82d9bap+1013, 0x1.3f266e198eac0p+959, -0x1.6aef86d06bb20p+904, 0x1.75ef2341f9b71p+850  }, // 1e305
        pow_table_entry{  0x1.6c8e5ca239029p+1016, -0x1.c43fd98036a41p+960, 0x1.d2a4bbdbcb0c4p+904, -0x1.64a89f6c3ed99p+850  }, // 1e306
        pow_table_entry{  0x1.c7b1f3cac7433p+1019, 0x1.cab0301fbbb2fp+963, -0x1.b8b2152d4230bp+907, -0x1.bdd2c7474e8ffp+853  }, // 1e307
        pow_table_entry{  0x1.1ccf385ebc8a0p+1023, -0x1.c2a3c3d855605p+966, -0x1.89b7a69e24af4p+912, 0x1.b4ae21b9b7730p+858  }, // 1e308
};

static_assert(
    sizeof(pow10_table) / sizeof(pow10_table[0]) ==
    static_cast<unsigned>(pow10_max_exponent - pow10_min_exponent + 1));

// Exact powers used by decimal parsing. The reciprocal for 5^n is
// ceil(2^binary_shift / 5^n); callers shorten it to the target precision
// before multiplying and then correct the candidate against the exact power.
inline constexpr std::uint64_t power5_u64[] = {
    1ull,
    5ull,
    25ull,
    125ull,
    625ull,
    3125ull,
    15625ull,
    78125ull,
    390625ull,
    1953125ull,
    9765625ull,
    48828125ull,
    244140625ull,
    1220703125ull,
    6103515625ull,
    30517578125ull,
    152587890625ull,
    762939453125ull,
    3814697265625ull,
    19073486328125ull,
    95367431640625ull,
    476837158203125ull,
    2384185791015625ull,
    11920928955078125ull,
    59604644775390625ull,
    298023223876953125ull,
    1490116119384765625ull,
    7450580596923828125ull
};

inline constexpr int power5_checkpoint_exponent = 13;

struct power5_checkpoint
{
    std::uint32_t words[11];
    std::uint8_t size;
};

inline constexpr power5_checkpoint power5_checkpoints[] = {
    { { 0x00000001u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }, 1 },
    { { 0x48c27395u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }, 1 },
    { { 0x320334b9u, 0x14adf4b7u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }, 2 },
    { { 0x88becaadu, 0x27128759u, 0x05e0a1fdu, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }, 3 },
    { { 0x8b31adb1u, 0xd0e54920u, 0x4957d300u, 0x01aba471u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }, 4 },
    { { 0xbd129b05u, 0x271ca2f7u, 0x4545f7a3u, 0x8e3fe1c8u, 0x00798b13u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }, 5 },
    { { 0x494178e9u, 0xcdcaa7d3u, 0x4c9b1e10u, 0xeadb8d5au, 0xc50b7f31u, 0x00228b6fu, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }, 6 },
    { { 0x34fe0a9du, 0x6b0f8581u, 0xc766ff00u, 0xd64283f9u, 0x47b62eb0u, 0xb2dcec0eu, 0x0009d174u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }, 7 },
    { { 0x299ab461u, 0xcbd35a82u, 0xafed1aa5u, 0xd95e18b9u, 0xb247b0b2u, 0x3f9d63b7u, 0xfb8c0314u, 0x0002ca5du, 0x00000000u, 0x00000000u, 0x00000000u }, 8 },
    { { 0xaf948f75u, 0xc603306au, 0xbaf0e4aeu, 0xbf11cf47u, 0x2cc56000u, 0xf5bfd307u, 0x551c5cadu, 0x0c8001abu, 0x0000cb09u, 0x00000000u, 0x00000000u }, 9 },
    { { 0x8c930e19u, 0x60d5f1e5u, 0x791290b5u, 0x766a4898u, 0xaf4f040fu, 0x24797cdcu, 0x7016455du, 0x3b076983u, 0xc40ab65bu, 0x000039b4u, 0x00000000u }, 10 },
    { { 0x97de6f8du, 0x57adcc28u, 0x7b7f423bu, 0x9011b7e6u, 0xf4d5f027u, 0x7e57e237u, 0x00f705c7u, 0x9cd7a93du, 0xc3e3efe8u, 0xac2d5daeu, 0x00001066u }, 11 }
};

inline constexpr int reciprocal_power5_precision = 268;
inline constexpr int reciprocal_power5_word_count = 9;
inline constexpr int reciprocal_power5_max_exponent = 72;

struct reciprocal_power5
{
    std::uint32_t words[reciprocal_power5_word_count];
    std::uint16_t binary_shift;
};

inline constexpr reciprocal_power5 reciprocal_power5_table[] = {
    { { 0xcccccccdu, 0xccccccccu, 0xccccccccu, 0xccccccccu, 0xccccccccu, 0xccccccccu, 0xccccccccu, 0xccccccccu, 0x00000cccu }, 270 },
    { { 0x0a3d70a4u, 0x3d70a3d7u, 0x70a3d70au, 0xa3d70a3du, 0xd70a3d70u, 0x0a3d70a3u, 0x3d70a3d7u, 0x70a3d70au, 0x00000a3du }, 272 },
    { { 0xd4fdf3b7u, 0x3126e978u, 0x5a1cac08u, 0x4fdf3b64u, 0x126e978du, 0xa1cac083u, 0xfdf3b645u, 0x26e978d4u, 0x00000831u }, 274 },
    { { 0x87fcb924u, 0x81d7dbf4u, 0xf6944673u, 0x4c985f06u, 0xea4a8c15u, 0x36113404u, 0x9652bd3cu, 0x71758e21u, 0x00000d1bu }, 277 },
    { { 0x0663c750u, 0x67dfe32au, 0x9210385cu, 0xd6e04c05u, 0x21d53cddu, 0xf80dc337u, 0x784230fcu, 0x5ac471b4u, 0x00000a7cu }, 279 },
    { { 0xd1e96c40u, 0x1fe64f54u, 0x41a6937du, 0x45803cd1u, 0x1b10fd7eu, 0xf9a49c2cu, 0xc69b5a63u, 0x7bd05af6u, 0x00000863u }, 281 },
    { { 0xb64246cdu, 0x330a1887u, 0x9c3db8c8u, 0x08cd2e1bu, 0x91b4c8cau, 0x29076046u, 0xa42bc3d3u, 0xf94d5e57u, 0x00000d6bu }, 284 },
    { { 0xf8350571u, 0x8f3b46d2u, 0x1697c706u, 0xd3d75816u, 0xa7c3d3d4u, 0x20d2b36bu, 0x1cefcfdcu, 0xc7711846u, 0x00000abcu }, 286 },
    { { 0x602a6ac1u, 0x7295d242u, 0xabac9f38u, 0x7645e011u, 0x53030fddu, 0x80a88f89u, 0x4a597316u, 0x05f4136bu, 0x00000897u }, 288 },
    { { 0xcd10aaceu, 0x8422ea03u, 0x12adcb8du, 0xf06fcce9u, 0x84d1b2fbu, 0x3440e5a8u, 0xdd5beb57u, 0x6fecebdeu, 0x00000dbeu }, 291 },
    { { 0xa40d5571u, 0x034f219cu, 0xa88b093eu, 0xf38ca3edu, 0x9d748f2fu, 0xf69a5153u, 0x4aafef78u, 0xbff0bcb2u, 0x00000afeu }, 293 },
    { { 0xe9a4445bu, 0x9c3f4e16u, 0xba08d431u, 0x8fa3b657u, 0xe45d3f59u, 0xf87b7442u, 0x088cbf93u, 0xccc096f5u, 0x000008cbu }, 295 },
    { { 0x42a06d5eu, 0x606549beu, 0x900e204fu, 0x4c3923bfu, 0x06fb988fu, 0x5a5f206bu, 0x40e13286u, 0xe13424bbu, 0x00000e12u }, 298 },
    { { 0x6880577eu, 0x805107cbu, 0x733e803fu, 0x09c74fccu, 0x38c946d9u, 0x484c19efu, 0xcd80f538u, 0x4dc35095u, 0x00000b42u }, 300 },
    { { 0x2066ac65u, 0x0040d309u, 0x8f653366u, 0x6e390ca3u, 0x60a1057au, 0xd37014bfu, 0x0acd90f9u, 0xd7cf73abu, 0x00000901u }, 302 },
    { { 0x670aad6fu, 0xcd3484dbu, 0x7f08523cu, 0x16c1add2u, 0x9a9b3bf7u, 0xebe68798u, 0xde15b4c2u, 0x594bec44u, 0x00000e69u }, 305 },
    { { 0x85a22459u, 0x70f6d0afu, 0x98d374fdu, 0xdf0157dbu, 0x487c2ff8u, 0xefeb9fadu, 0x4b44909bu, 0x7aa3236au, 0x00000b87u }, 307 },
    { { 0x9e1b5047u, 0xc0c573bfu, 0x470f90cau, 0x7f344649u, 0x39fcf32du, 0xf322e624u, 0xd5d073afu, 0x2ee8e921u, 0x00000939u }, 309 },
    { { 0xfcf88071u, 0x346f1f98u, 0x3e7f4e11u, 0x65207075u, 0xc32e51e2u, 0x1e9e3d06u, 0x561a52b3u, 0xe4a7db69u, 0x00000ec1u }, 312 },
    { { 0xfd9399f4u, 0x9058e613u, 0x6532a4dau, 0xea8059f7u, 0xcf584181u, 0x4bb1ca6bu, 0x11aea88fu, 0x50864921u, 0x00000bceu }, 314 },
    { { 0x97a947f7u, 0x737a51a9u, 0xea8eea48u, 0xeecd14c5u, 0xa5e03467u, 0x6fc16ebcu, 0xa7beed3fu, 0xda05074du, 0x00000971u }, 316 },
    { { 0x8c420cbeu, 0x1f2a1c42u, 0xaa7e43a7u, 0xb148213cu, 0xd633870cu, 0x4c68b12du, 0x72cb1532u, 0x90080bafu, 0x00000f1cu }, 319 },
    { { 0x09ce7098u, 0x18ee7d02u, 0x886502ecu, 0xf439b430u, 0x782938d6u, 0x7053c0f1u, 0x28a2775bu, 0xd9a00959u, 0x00000c16u }, 321 },
    { { 0x6e3ec07au, 0x472530ceu, 0xa050cf23u, 0xc3615cf3u, 0x93542d78u, 0x26a96727u, 0x53b52c49u, 0xe14cd447u, 0x000009abu }, 323 },
    { { 0x49fe00c2u, 0xd83b814au, 0x33b47e9eu, 0x389bc7ecu, 0x522048c1u, 0x3ddbd83fu, 0xec5513a8u, 0x687aed3eu, 0x00000f79u }, 326 },
    { { 0x07fe6702u, 0x1362cdd5u, 0x8fc3987fu, 0xc6e30656u, 0xa819d3cdu, 0x64afe032u, 0x89dda953u, 0x20625765u, 0x00000c61u }, 328 },
    { { 0x39985268u, 0x42b57177u, 0x0c9c79ffu, 0x9f1c0512u, 0x5347dca4u, 0xea264cf5u, 0x07e48775u, 0x4d1b791eu, 0x000009e7u }, 330 },
    { { 0xf5c083d9u, 0x9def1bf1u, 0xe0fa5ccbu, 0xcb60081cu, 0x853fc76du, 0xa9d6e188u, 0x0ca0d8bcu, 0x7b5f2830u, 0x00000fd8u }, 333 },
    { { 0xf7cd3647u, 0x7e58e327u, 0x80c84a3cu, 0x6f80067du, 0x043305f1u, 0xee45813au, 0xa3b3e096u, 0x2f7f5359u, 0x00000cadu }, 335 },
    { { 0xf970f839u, 0x98471c1fu, 0x00a03b63u, 0xf2ccd1feu, 0xd028d18du, 0x58379a94u, 0x4fc31a12u, 0x25ff75e1u, 0x00000a24u }, 337 },
    { { 0xc78d9361u, 0xe038e34cu, 0x00802f82u, 0x8f0a4198u, 0x0ced7471u, 0xacf94877u, 0x3fcf480eu, 0xeb32c4b4u, 0x0000081cu }, 339 },
    { { 0xd8e28568u, 0x338e387au, 0xcd99e59eu, 0xb1aa028cu, 0x14af20b5u, 0xe18eda58u, 0x994ba67du, 0x11ead453u, 0x00000cfbu }, 342 },
    { { 0xe0b53786u, 0x8fa4f9fbu, 0x0ae1847eu, 0xc154ced7u, 0xdd58e6f7u, 0x1ad8aeacu, 0xadd61ecbu, 0x74bbdd0fu, 0x00000a62u }, 344 },
    { { 0xe6f75f9fu, 0x0c83fb2fu, 0x3be79d32u, 0x67770bdfu, 0xb113ebf9u, 0xaf13bef0u, 0x24ab4bd5u, 0xc3c97da6u, 0x0000084eu }, 346 },
    { { 0x0b2565cbu, 0x14065eb3u, 0x2ca5c850u, 0xd8be7965u, 0x4e86465bu, 0xe4ec64b4u, 0x07787955u, 0xd2dbfc3du, 0x00000d4au }, 349 },
    { { 0x08eab7d5u, 0xdcd1e55cu, 0xbd516d0cu, 0xe098611du, 0x0b9e9eafu, 0x50bd1d5du, 0x392d2ddeu, 0x42499697u, 0x00000aa2u }, 351 },
    { { 0xa0bbc644u, 0x4a418449u, 0x6441240au, 0xe6e04db1u, 0xa2e54bbfu, 0x0d64177du, 0xc75757e5u, 0xcea14545u, 0x00000881u }, 353 },
    { { 0x012c706du, 0x1068d3a9u, 0xa06839aau, 0xa49a15e8u, 0x04a212ccu, 0x7bd358c9u, 0x7225596eu, 0x7dced53cu, 0x00000d9cu }, 356 },
    { { 0xcdbd26beu, 0xd9ed7620u, 0x80536154u, 0xb6e1ab20u, 0xd081a8a3u, 0xc975e0a0u, 0xc1b77abeu, 0x97d8aa96u, 0x00000ae3u }, 358 },
    { { 0x0afdb898u, 0xe18ac4e7u, 0x0042b443u, 0x2be7bc1au, 0xda0153b6u, 0x3ac4b3b3u, 0xce2c6232u, 0x1313bbabu, 0x000008b6u }, 360 },
    { { 0xab2f8dc0u, 0x0277a171u, 0xcd3786d3u, 0x463f935cu, 0xf66885f0u, 0xc46dec52u, 0xe37a36b6u, 0x1e85f912u, 0x00000df0u }, 363 },
    { { 0x55bfa499u, 0xcec61ac1u, 0x70f938a8u, 0x6b660f7du, 0x91ed37f3u, 0x038b2375u, 0x1c61c55fu, 0x7ed1940fu, 0x00000b26u }, 365 },
    { { 0x4499507bu, 0x3f04e234u, 0xf3fa93bau, 0xbc51a5fdu, 0xdb242cc2u, 0x3608e92au, 0xb04e377fu, 0x98a7a9a5u, 0x000008ebu }, 367 },
    { { 0x6dc21a5eu, 0x64d49d20u, 0xecc41f90u, 0x6082a32fu, 0xc506ae04u, 0x89a7db77u, 0xb3b058cbu, 0xc10c42a2u, 0x00000e45u }, 370 },
    { { 0x8b01aeb2u, 0xb7107db3u, 0x8a367fa6u, 0x4d354f59u, 0xd0d224d0u, 0x07b97c5fu, 0x5c8d13d6u, 0x00d69bb5u, 0x00000b6bu }, 372 },
    { { 0xd59af228u, 0x927397c2u, 0x6e91ffb8u, 0xd75dd914u, 0xa70e83d9u, 0x3961304cu, 0xb070dcabu, 0x6712162au, 0x00000922u }, 374 },
    { { 0xbc2b1d0cu, 0xb71f5937u, 0x174fff8du, 0xbefc8e87u, 0x0b4a6c8fu, 0x8f01e6e1u, 0xe71afaabu, 0x71b689ddu, 0x00000e9du }, 377 },
    { { 0x30227da3u, 0xc5b2adc6u, 0xdf7332d7u, 0x3263a538u, 0xd5d523a6u, 0x0c018580u, 0xec159556u, 0x27c53b17u, 0x00000bb1u }, 379 },
    { { 0x8ce86483u, 0x37c2249eu, 0xe5f5c246u, 0x5b82ea93u, 0x44aa82ebu, 0x7001379au, 0x89aadddeu, 0x86376279u, 0x0000095au }, 381 },
    { { 0xe173d404u, 0x8c69d430u, 0xd65603a3u, 0xc59e441fu, 0xd44404abu, 0xe66858f6u, 0x0f77c963u, 0x3d256a5cu, 0x00000ef7u }, 384 },
    { { 0xe78fdcd0u, 0xa387dcf3u, 0x11de694fu, 0xd14b69b3u, 0xdd0336efu, 0x8520472bu, 0xa5f96de9u, 0xfdb78849u, 0x00000bf8u }, 386 },
    { { 0x1fa64a40u, 0x1c6cb0c3u, 0x417ebaa6u, 0x743c548fu, 0xe4029259u, 0x6a8038efu, 0xb7fabe54u, 0xfe2c6d07u, 0x00000993u }, 388 },
    { { 0x990a1067u, 0x60ade79eu, 0xcf312aa3u, 0x5393ba7eu, 0x066a83c2u, 0xdd99f4b3u, 0x265dfd53u, 0x304714d9u, 0x00000f53u }, 391 },
    { { 0x473b4052u, 0xb3be52e5u, 0x3f5a8882u, 0x42dc9532u, 0x6b886968u, 0x7e14c3c2u, 0x51e4caa9u, 0x8d05aa47u, 0x00000c42u }, 393 },
    { { 0x05c90042u, 0x5c984251u, 0xcc486d35u, 0xcf16ddc1u, 0x5606bab9u, 0x64dd6968u, 0x4183d554u, 0xd737bb6cu, 0x000009ceu }, 395 },
    { { 0xd60e66cfu, 0xc75a03b4u, 0xad40aebbu, 0xe4f162cfu, 0x56712ac2u, 0x6e2f0f0du, 0x68d2eeedu, 0x58592be0u, 0x00000fb1u }, 398 },
    { { 0xde71ebd9u, 0x3914cfc3u, 0xbdcd5896u, 0xb727823fu, 0x45275568u, 0x5825a5a4u, 0x20a8bf24u, 0xe047564du, 0x00000c8du }, 400 },
    { { 0xb1f4bcaeu, 0xfa770c9cu, 0xcb0aad44u, 0xc5b934ffu, 0xd0ec4453u, 0xaceaeae9u, 0xe6ed65b6u, 0x19d2ab70u, 0x00000a0bu }, 402 },
    { { 0x27f6fd58u, 0xc85f3d4au, 0x6f3bbdd0u, 0x37c75d99u, 0x40bd0376u, 0xbd88bbeeu, 0xebf11e2bu, 0xe17555f3u, 0x00000808u }, 404 },
    { { 0xd98b2ef3u, 0xda31fba9u, 0x185f961au, 0xbfa5628fu, 0x6794d256u, 0x2f412cb0u, 0x131b6379u, 0x02555653u, 0x00000cdbu }, 407 },
    { { 0x7ad5bf29u, 0xe1c19621u, 0x46b2de7bu, 0x32eab53fu, 0x52dd7512u, 0x8c3423c0u, 0xa8e2b5fau, 0xceaaab75u, 0x00000a48u }, 409 },
    { { 0x624498eeu, 0xe7ce11b4u, 0x055be52fu, 0x8f222a99u, 0x424ac40eu, 0x3cf68300u, 0x53e89195u, 0x3eeeef91u, 0x0000083au }, 411 },
    { { 0x03a0f4afu, 0x72e34f87u, 0xd55fd519u, 0x7e9d10f4u, 0x03aad34au, 0xc7f0d19au, 0x8640e8eeu, 0xfe4b18e8u, 0x00000d29u }, 414 },
    { { 0xcfb3f6f3u, 0x28b5d938u, 0xaab310e1u, 0xfee40d90u, 0x362242a1u, 0x398d747bu, 0x9e9a53f2u, 0xfea27a53u, 0x00000a87u }, 416 },
    { { 0x0c8ff8c2u, 0x53c4adc7u, 0xbbc273e7u, 0xcbe9a473u, 0x2b4e9bb4u, 0x947129fcu, 0x4baea98eu, 0xcbb52ea9u, 0x0000086cu }, 418 },
    { { 0xe0e65ad0u, 0xb93aafa4u, 0x2c6a530bu, 0x4642a0b9u, 0xabb0f921u, 0xed81dcc6u, 0x791775b0u, 0xdf884aa8u, 0x00000d7au }, 421 },
    { { 0x1a51e240u, 0x60fbbfb7u, 0xbd21dc09u, 0x05021a2du, 0x22f3fa81u, 0x2467e3d2u, 0x2dac5e27u, 0xb2d36eedu, 0x00000ac8u }, 423 },
    { { 0x7b74b500u, 0xe72fcc92u, 0x641b166du, 0x6a6814f1u, 0xb58ffb9au, 0xe9ecb641u, 0xf156b1b8u, 0x8f0f8bf0u, 0x000008a0u }, 425 },
    { { 0xc5878800u, 0xa5194750u, 0x0691bd7cu, 0x770cee4fu, 0x88e65f5du, 0x431456cfu, 0x82244f8eu, 0xb1b27981u, 0x00000dcdu }, 428 },
    { { 0x379fa000u, 0x50e105dau, 0x9edafdfdu, 0x9270bea5u, 0x6d851917u, 0x35a9df0cu, 0xce8372d8u, 0xf48ec79au, 0x00000b0au }, 430 },
    { { 0xc6194ccdu, 0x73e737e1u, 0x18af3197u, 0xa85a321eu, 0x8ad0e0dfu, 0xc487e5a3u, 0xa535f579u, 0x90723948u, 0x000008d5u }, 432 },
    { { 0xd68ee148u, 0xb971f302u, 0xc1184f58u, 0xda29e9c9u, 0xde1b0165u, 0x6da63c38u, 0xd523225cu, 0x80b6c20du, 0x00000e22u }, 435 }
};

static_assert(
    sizeof(reciprocal_power5_table) / sizeof(reciprocal_power5_table[0]) ==
    static_cast<unsigned>(reciprocal_power5_max_exponent));

} // namespace pow_tables
} // namespace bl::detail

#endif // FLTX_DETAIL_POW_TABLES_INCLUDED
