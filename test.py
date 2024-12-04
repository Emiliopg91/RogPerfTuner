import math

steps = 100
for i in range(steps+1):
    x = math.radians((i/steps)*90)
    y = math.sin(x)
    print(f"{x}: {y*255}")