//
// Created by lucien on 1/29/19.
//

#ifndef HALF_HALF_H
#define HALF_HALF_H

namespace Eigen {
    namespace half_impl {
        struct __half_raw {
            explicit __half_raw(unsigned short raw = 0) : x(raw) {}
            unsigned short x;
        };

        struct half_base : public __half_raw {
            half_base() {}
            half_base(const half_base &tmp) : __half_raw(tmp) {}
            half_base(const __half_raw &tmp) : __half_raw(tmp) {}
        };

        union FP32 {
            unsigned int u;
            float f;
        };

        __half_raw float_to_half_rtne(float ff) {
            FP32 f;
            f.f = ff;

            const FP32 f32infty = { 255 << 23 }; // 0xff
            const FP32 f16max = { (127 + 16) << 23 }; // 0x8f
            const FP32 denorm_magic = { ((127 - 15) + (23 - 10) + 1) << 23 }; // 0x7e
            unsigned int sign_mask = 0x80000000u;
            __half_raw o;
            o.x = static_cast<unsigned short>(0x0u);

            unsigned int sign = f.u & sign_mask;
            f.u ^= sign;

            // NOTE all the integer compares in this function can be safely
            // compiled into signed compares since all operands are below
            // 0x80000000. Important if you want fast straight SSE2 code
            // (since there's no unsigned PCMPGTD).
            if (f.u >= f16max.u) {  // result is Inf or NaN (all exponent bits set)
                o.x = (f.u > f32infty.u) ? 0x7e00 : 0x7c00; // NaN->qNaN and Inf->Inf
            } else {  // (De)normalized number or zero
                if (f.u < (113 << 23)) { // 0x71 - 0x7f = -14 smallest positive exponent
                    // resulting FP16 is subnormal or zero
                    // use a magic value to align our 10 mantissa bits at the bottom of
                    // the float. as long as FP addition is round-to-nearest-even this
                    // just works.
                    f.f += denorm_magic.f; // denorm_magic = 0.5

                    // and one integer subtract of the bias later, we have our final float!
                    o.x = static_cast<unsigned short>(f.u - denorm_magic.u);
                } else {
                    unsigned int mant_odd = (f.u >> 13) & 1; // resulting mantissa is odd

                    // update exponent, rounding bias part 1
                    f.u += ((unsigned int)(15 - 127) << 23) + 0xfff;
                    // rounding bias part 2
                    f.u += mant_odd;
                    // take the bits!
                    o.x = static_cast<unsigned short>(f.u >> 13);
                }
            }
            o.x |= static_cast<unsigned short>(sign >> 16);
            return o;
        }

        float half_to_float(__half_raw h) {
            const FP32 magic = {113 << 23};
            const unsigned int shifted_exp = 0x7c00 << 13; // exponent mask after shift
            FP32 o;

            o.u = (h.x & 0x7fff) << 13;             // exponent/mantissa bits
            unsigned int exp = shifted_exp & o.u;   // just the exponent
            o.u += (127 - 15) << 23;                // exponent adjust

            // handle exponent special cases
            if (exp == shifted_exp) {     // Inf/NaN?
                o.u += (128 - 16) << 23;    // extra exp adjust
            } else if (exp == 0) {        // Zero/Denormal?
                o.u += 1 << 23;             // extra exp adjust
                o.f -= magic.f;             // renormalize
            }

            o.u |= (h.x & 0x8000) << 16;    // sign bit
            return o.f;
        }
    } // namespace half_impl

    struct half : public half_impl::half_base {
        using __half_raw = half_impl::__half_raw;
        half() {}
        half(const __half_raw &tmp) : half_impl::half_base(tmp) {}
        half(const half &tmp) : half_impl::half_base(tmp) {}
        explicit operator float() const {
            return half_impl::half_to_float(*this);
        }
        explicit half(float f) : half_impl::half_base(half_impl::float_to_half_rtne(f)) {}
    };

} // namespace Eigen

#endif //HALF_HALF_H
