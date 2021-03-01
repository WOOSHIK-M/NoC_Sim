import numpy as np

lut = np.zeros([512, 9])

lut[:10, 0] = 4
lut[:10, 1] = 4

np.savetxt("data/test.dat", lut)
