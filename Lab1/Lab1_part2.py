# Kyle Johnsen
# Lab 1, Part 2

import matplotlib.pyplot as plt

# Test values
V1, R1, R2, R3, R4, R5, R6 = 5, 5, 10, 10, 20, 10, 15

# Find the equivalent resistance
R_246 = R2+R4+R6
R_3_246 = 1/(1/R3 + 1/R_246)
R_EQ = R_3_246 + R1 + R5

# Use the equivalent resistance to figure out the output current
I_OUT = V1 / R_EQ

# Apply KCL and Ohm's Law to Figure out the rest
I_R1 = I_R5 = I_OUT
V_R3 = V1 - I_R1*R1 - I_R5*R5
I_R3 = V_R3/R3
I_R2 = I_R4 = I_R6 = V_R3 / R_246

# Compute power from current and resistance 
P_R1 = R1*I_R1**2
P_R2 = R2*I_R2**2
P_R3 = R3*I_R3**2
P_R4 = R4*I_R4**2
P_R5 = R5*I_R5**2
P_R6 = R6*I_R6**2

# Print the current and power to 3 decimal points
print(f'''Current (Amps) and Power (Watts) for each resistor
({I_R1:.3f}, {P_R1:.3f})
({I_R2:.3f}, {P_R2:.3f})
({I_R3:.3f}, {P_R3:.3f})
({I_R4:.3f}, {P_R4:.3f})
({I_R5:.3f}, {P_R5:.3f})
({I_R6:.3f}, {P_R6:.3f})''')

