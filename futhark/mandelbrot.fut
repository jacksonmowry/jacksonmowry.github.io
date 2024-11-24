type complex = (f64, f64)

def mandelbrot_step ((Zn_r, Zn_i): complex)
                    ((C_r, C_i): complex)
                  : complex =
  let real_part = Zn_r*Zn_r - Zn_i*Zn_i + C_r
  let imag_part = 2.0*Zn_r*Zn_i + C_i
  in (real_part, imag_part)
