#include <concepts>
#include <cstdint>
#include <iostream>
#include <type_traits>

#include <fltx.h>
using namespace bl;
using namespace bl::types;
using namespace bl::literals;
using namespace bl::int_literals;

/*

bl::pow precision policy:

  Follows std::pow rules: integer arguments count as f64 for overload selection, so these return f64:
    - bl::pow(10, 10)
    - bl::pow(10.0f, 3)
    - bl::pow(10, 3.0f)

  fdd/fqd act as wider floating types:
    - fdd wins over f64
    - fqd wins over fdd


bl::ipow precision policy:

  - Keeps the base type as the result type.
  - Debug builds assert when result overflows base type
  - Release builds silently wrap

*/


#define assert_value_type(value, type) \
    static_assert(std::same_as<std::remove_cvref_t<decltype(value)>, type>)

consteval void consteval_pow_tests()
{
    // pow(integer, ...): integers count as f64 for overload ranking.
    constexpr auto a0 = bl::pow(4_u8, 2_u8);           assert_value_type(a0, f64);
    constexpr auto a1 = bl::pow(10_u8, 10_i16);        assert_value_type(a1, f64);
    constexpr auto a2 = bl::pow(2_u8, 7_i32);          assert_value_type(a2, f64);
    constexpr auto a3 = bl::pow(2_u8, 7);              assert_value_type(a3, f64);
    constexpr auto a4 = bl::pow(4_u8, 3.0f);           assert_value_type(a4, f64);
    constexpr auto a5 = bl::pow(10_u8, 12.0);          assert_value_type(a5, f64);
    constexpr auto a6 = bl::pow(10_u8, 12.0_dd);       assert_value_type(a6, fdd);
    constexpr auto a7 = bl::pow(10_u8, 12.0_qd);       assert_value_type(a7, fqd);
                                                       
    constexpr auto b0 = bl::pow(10_i16, 4_u8);         assert_value_type(b0, f64);
    constexpr auto b1 = bl::pow(10_i16, 4_i16);        assert_value_type(b1, f64);
    constexpr auto b2 = bl::pow(10_i16, 4_i32);        assert_value_type(b2, f64);
    constexpr auto b3 = bl::pow(10_i16, 4);            assert_value_type(b3, f64);
    constexpr auto b4 = bl::pow(10_i16, 12.0f);        assert_value_type(b4, f64);
    constexpr auto b5 = bl::pow(10_i16, 12.0);         assert_value_type(b5, f64);
    constexpr auto b6 = bl::pow(10_i16, 12.0_dd);      assert_value_type(b6, fdd);
    constexpr auto b7 = bl::pow(10_i16, 12.0_qd);      assert_value_type(b7, fqd);
                                                       
    constexpr auto c0 = bl::pow(10, 9_u8);             assert_value_type(c0, f64);
    constexpr auto c1 = bl::pow(10, 10);               assert_value_type(c1, f64);
    constexpr auto c2 = bl::pow(10, 12.0f);            assert_value_type(c2, f64);
    constexpr auto c3 = bl::pow(10, 12.0);             assert_value_type(c3, f64);
    constexpr auto c4 = bl::pow(10, 12.0_dd);          assert_value_type(c4, fdd);
    constexpr auto c5 = bl::pow(10, 12.0_qd);          assert_value_type(c5, fqd);

    // pow(floating, ...): f32 + integer returns f64, matching std::pow.
    constexpr auto d0 = bl::pow(10.0f, 12.0f);         assert_value_type(d0, f32);
    constexpr auto d1 = bl::pow(10.0f, 12_u8);         assert_value_type(d1, f64);
    constexpr auto d2 = bl::pow(10.0f, 12);            assert_value_type(d2, f64);
    constexpr auto d3 = bl::pow(10.0f, 12_u64);        assert_value_type(d3, f64);
    constexpr auto d4 = bl::pow(10.0f, 12.0);          assert_value_type(d4, f64);
    constexpr auto d5 = bl::pow(10.0f, 12.0_dd);       assert_value_type(d5, fdd);
    constexpr auto d6 = bl::pow(10.0f, 12.0_qd);       assert_value_type(d6, fqd);
                                                       
    constexpr auto e0 = bl::pow(10.0, 12.0f);          assert_value_type(e0, f64);
    constexpr auto e1 = bl::pow(10.0, 12_u8);          assert_value_type(e1, f64);
    constexpr auto e2 = bl::pow(10.0, 12);             assert_value_type(e2, f64);
    constexpr auto e3 = bl::pow(10.0, 12_u64);         assert_value_type(e3, f64);
    constexpr auto e4 = bl::pow(10.0, 12.0);           assert_value_type(e4, f64);
    constexpr auto e5 = bl::pow(10.0, 12.0_dd);        assert_value_type(e5, fdd);
    constexpr auto e6 = bl::pow(10.0, 12.0_qd);        assert_value_type(e6, fqd);
                                                       
    constexpr auto f0 = bl::pow(10.0_dd, 12.0f);       assert_value_type(f0, fdd);
    constexpr auto f1 = bl::pow(10.0_dd, 12_u8);       assert_value_type(f1, fdd);
    constexpr auto f2 = bl::pow(10.0_dd, 12);          assert_value_type(f2, fdd);
    constexpr auto f3 = bl::pow(10.0_dd, 12_u64);      assert_value_type(f3, fdd);
    constexpr auto f4 = bl::pow(10.0_dd, 12.0);        assert_value_type(f4, fdd);
    constexpr auto f5 = bl::pow(10.0_dd, 12.0_dd);     assert_value_type(f5, fdd);
    constexpr auto f6 = bl::pow(10.0_dd, 12.0_qd);     assert_value_type(f6, fqd);
                                                        
    constexpr auto g0 = bl::pow(10.0_qd, 12.0f);       assert_value_type(g0, fqd);
    constexpr auto g1 = bl::pow(10.0_qd, 12_u8);       assert_value_type(g1, fqd);
    constexpr auto g2 = bl::pow(10.0_qd, 12);          assert_value_type(g2, fqd);
    constexpr auto g3 = bl::pow(10.0_qd, 12_u64);      assert_value_type(g3, fqd);
    constexpr auto g4 = bl::pow(10.0_qd, 12.0);        assert_value_type(g4, fqd);
    constexpr auto g5 = bl::pow(10.0_qd, 12.0_dd);     assert_value_type(g5, fqd);
    constexpr auto g6 = bl::pow(10.0_qd, 12.0_qd);     assert_value_type(g6, fqd);

    static_assert(a1 == 10000000000.0);
    static_assert(c1 == 10000000000.0);

    // negative_exponent_overloads
    {
        constexpr auto a0 = bl::pow(2_u8, -3_i8);         assert_value_type(a0, f64);
        constexpr auto a1 = bl::pow(2_u8, -5_i16);        assert_value_type(a1, f64);
        constexpr auto a2 = bl::pow(2_i16, -7_i32);       assert_value_type(a2, f64);
        constexpr auto a3 = bl::pow(2, -9_i64);           assert_value_type(a3, f64);
        constexpr auto a4 = bl::pow(2, -3.0f);            assert_value_type(a4, f64);
        constexpr auto a5 = bl::pow(2, -3.0);             assert_value_type(a5, f64);
        constexpr auto a6 = bl::pow(2, -3.0_dd);          assert_value_type(a6, fdd);
        constexpr auto a7 = bl::pow(2, -3.0_qd);          assert_value_type(a7, fqd);
                                                          
        constexpr auto b0 = bl::pow(2.0f, -3_i8);         assert_value_type(b0, f64);
        constexpr auto b1 = bl::pow(2.0f, -7_i32);        assert_value_type(b1, f64);
        constexpr auto b2 = bl::pow(2.0f, -3.0f);         assert_value_type(b2, f32);
        constexpr auto b3 = bl::pow(2.0f, -3.0);          assert_value_type(b3, f64);
        constexpr auto b4 = bl::pow(2.0f, -3.0_dd);       assert_value_type(b4, fdd);
        constexpr auto b5 = bl::pow(2.0f, -3.0_qd);       assert_value_type(b5, fqd);
                                                          
        constexpr auto c0 = bl::pow(2.0, -3_i8);          assert_value_type(c0, f64);
        constexpr auto c1 = bl::pow(2.0, -7_i32);         assert_value_type(c1, f64);
        constexpr auto c2 = bl::pow(2.0, -3.0f);          assert_value_type(c2, f64);
        constexpr auto c3 = bl::pow(2.0, -3.0);           assert_value_type(c3, f64);
        constexpr auto c4 = bl::pow(2.0, -3.0_dd);        assert_value_type(c4, fdd);
        constexpr auto c5 = bl::pow(2.0, -3.0_qd);        assert_value_type(c5, fqd);
                                                          
        constexpr auto d0 = bl::pow(2.0_dd, -3_i8);      assert_value_type(d0, fdd);
        constexpr auto d1 = bl::pow(2.0_dd, -5_i16);     assert_value_type(d1, fdd);
        constexpr auto d2 = bl::pow(2.0_dd, -7_i32);     assert_value_type(d2, fdd);
        constexpr auto d3 = bl::pow(2.0_dd, -9_i64);     assert_value_type(d3, fdd);
        constexpr auto d4 = bl::pow(2.0_dd, -3.0f);      assert_value_type(d4, fdd);
        constexpr auto d5 = bl::pow(2.0_dd, -3.0);       assert_value_type(d5, fdd);
        constexpr auto d6 = bl::pow(2.0_dd, -3.0_dd);    assert_value_type(d6, fdd);
        constexpr auto d7 = bl::pow(2.0_dd, -3.0_qd);    assert_value_type(d7, fqd);
                                                          
        constexpr auto e0 = bl::pow(2.0_qd, -3_i8);      assert_value_type(e0, fqd);
        constexpr auto e1 = bl::pow(2.0_qd, -5_i16);     assert_value_type(e1, fqd);
        constexpr auto e2 = bl::pow(2.0_qd, -7_i32);     assert_value_type(e2, fqd);
        constexpr auto e3 = bl::pow(2.0_qd, -9_i64);     assert_value_type(e3, fqd);
        constexpr auto e4 = bl::pow(2.0_qd, -3.0f);      assert_value_type(e4, fqd);
        constexpr auto e5 = bl::pow(2.0_qd, -3.0);       assert_value_type(e5, fqd);
        constexpr auto e6 = bl::pow(2.0_qd, -3.0_dd);    assert_value_type(e6, fqd);
        constexpr auto e7 = bl::pow(2.0_qd, -3.0_qd);    assert_value_type(e7, fqd);
                                                          
        constexpr auto f0 = bl::pow(-2_i16, -3_i8);      assert_value_type(f0, f64);
        constexpr auto f1 = bl::pow(-2.0_dd, -5_i16);    assert_value_type(f1, fdd);
        constexpr auto f2 = bl::pow(-2.0_qd, -7_i32);    assert_value_type(f2, fqd);

        static_assert(a0 == 0.125);
        static_assert(a1 == 0.03125);
        static_assert(a2 == 0.0078125);
        static_assert(a3 == 0.001953125);
        static_assert(d0 == 0.125_dd);
        static_assert(e0 == 0.125_qd);
        static_assert(f0 == -0.125);
    }

    // exponent_value_ranges
    {
        // These use deliberately different exponent storage types with values chosen
        // to exercise the small-exp switch, its boundary, and the generic fallback.
        constexpr auto a0 = bl::pow(1.25, 12_u8);                   assert_value_type(a0, f64);
        constexpr auto a1 = bl::pow(1.25, 12_u64);                  assert_value_type(a1, f64);
        constexpr auto a2 = bl::pow(1.001, 255_u64);                assert_value_type(a2, f64);
        constexpr auto a3 = bl::pow(1.001, 256_u64);                assert_value_type(a3, f64);
        constexpr auto a4 = bl::pow(1.0001, 4096_u64);              assert_value_type(a4, f64);
        constexpr auto a5 = bl::pow(1.00001, 65536_u64);            assert_value_type(a5, f64);
        constexpr auto a6 = bl::pow(1.0000001_qd, 12345678_u64);    assert_value_type(a6, fqd);
                                                                    
        constexpr auto b0 = bl::pow(2.0_dd, -12_i8);                assert_value_type(b0, fdd);
        constexpr auto b1 = bl::pow(2.0_dd, -12_i64);               assert_value_type(b1, fdd);
                                                                    
        constexpr auto c0 = bl::pow(2.0f, 120_i32);                 assert_value_type(c0, f64);
        constexpr auto c1 = bl::pow(2.0, 512_i32);                  assert_value_type(c1, f64);
        constexpr auto c2 = bl::pow(2.0_dd, 512_i32);               assert_value_type(c2, fdd);
        constexpr auto c3 = bl::pow(2.0_qd, 512_i32);               assert_value_type(c3, fqd);
                                                                    
        constexpr auto d0 = bl::pow(10.0f, 30_i32);                 assert_value_type(d0, f64);
        constexpr auto d1 = bl::pow(10.0, 30_i32);                  assert_value_type(d1, f64);
        constexpr auto d2 = bl::pow(10.0_dd, 30_i32);               assert_value_type(d2, fdd);
        constexpr auto d3 = bl::pow(10.0_qd, 30_i32);               assert_value_type(d3, fqd);
        constexpr auto d4 = bl::pow(20.0_dd, 30_i32);               assert_value_type(d4, fdd);
        constexpr auto d5 = bl::pow(125.0_qd, 24_i32);              assert_value_type(d5, fqd);
                                                                     
        constexpr auto e0 = bl::pow(10.0_dd, -30_i32);              assert_value_type(e0, fdd);
        constexpr auto e1 = bl::pow(10.0_qd, -30_i32);              assert_value_type(e1, fqd);
        constexpr auto e2 = bl::pow(-20.0_dd, -30_i32);             assert_value_type(e2, fdd);

        static_assert(d4 == bl::pow(fdd{ 20 }, 30_i32));
        static_assert(d5 == bl::pow(fqd{ 125 }, 24_i32));
    }
}


consteval void consteval_ipow_tests()
{
    // ipow(int-like, int-like): default return is the base type.
    constexpr auto a0 = bl::ipow(4_u8, 2_u8);       assert_value_type(a0, u8);
    constexpr auto a1 = bl::ipow(4_u8, 3_i16);      assert_value_type(a1, u8);
    constexpr auto a2 = bl::ipow(2_u8, 7_i32);      assert_value_type(a2, u8);
    constexpr auto a3 = bl::ipow(2_u8, 7);          assert_value_type(a3, u8);

    constexpr auto b0 = bl::ipow(10_i16, 4_u8);     assert_value_type(b0, i16);
    constexpr auto b1 = bl::ipow(10_i16, 4_i16);    assert_value_type(b1, i16);
    constexpr auto b2 = bl::ipow(10_i16, 4_i32);    assert_value_type(b2, i16);
    constexpr auto b3 = bl::ipow(10_i16, 4);        assert_value_type(b3, i16);

    constexpr auto c0 = bl::ipow(10, 9_u8);         assert_value_type(c0, int);
    constexpr auto c1 = bl::ipow(10, 9);            assert_value_type(c1, int);

    #if 0 // Expected failures
    {
        // Integral ipow keeps the base type as the result type.
        // Overflow is diagnosed during constant evaluation in all builds.
        // Runtime overflow is checked only in Debug; Release runtime calls wrap silently.
        constexpr auto a0 = bl::ipow(10_i16, 5_i16);
        constexpr auto a1 = bl::ipow(10, 10);
    }
    #endif
}

int main()
{
    consteval_pow_tests();
    consteval_ipow_tests();

    return 0;
}
