#include <cmath>
#include <cstdint>

extern "C" {

// lm: pointer to 42 floats: (x,y) for each of the 21 landmarks, in order  
// is_right: 1 if Right hand, 0 if Left  
// out: pointer to 5 ints; will be filled with 0 or 1 for each finger  
void fingers_up(const float* lm, uint8_t is_right, uint8_t* out) {
    // TIP_IDS = {4, 8, 12, 16, 20}
    // thumb: compare x of tip vs x of tip-2
    float tip_x = lm[4*2 + 0], base_x = lm[(4-2)*2 + 0];  
    bool thumb_up = ((tip_x < base_x) == (is_right!=0));  
    out[0] = thumb_up ? 1 : 0;

    // other four fingers
    for (int idx = 1; idx < 5; ++idx) {
        int i = (4 + idx*4)*2;      // tip index: 8,12,16,20
        int j = (4 + idx*4 - 2)*2;  // tip-2: 6,10,14,18
        float y_tip  = lm[i+1];
        float y_base = lm[j+1];
        out[idx] = (y_tip < y_base) ? 1 : 0;
    }
}

// Given (x1,y1),(x2,y2), return slope, or ±999 if dx≈0
double calc_slope(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1, dy = y2 - y1;
    if (std::fabs(dx) > 1e-3f) return double(dy/dx);
    return (dy > 0 ? 999.0 : -999.0);
}

} // extern "C"
