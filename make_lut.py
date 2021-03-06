import numpy as np

lut = np.zeros([512, 16])

lut[:2, 3] = 7
lut[:2, 6] = 13
lut[:2, 12] = 10
lut[:2, 9] = 4

# lut[:10, 0] = 11
# lut[:10, 5] = 1
# lut[:10, 10] = 16
# lut[:10, 15] = 6

np.savetxt("data/test.dat", lut)
