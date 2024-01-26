% Kyle Johnsen
% Lab 1, Part 2

% Test values
V1 = 5; 
R1 = 5; R2 = 10; R3 = 10; R4 = 20; R5= 10; R6=15;

% Find the equivalent resistance
R_246 = R2+R4+R6;
R_3_246 = 1/(1/R3 + 1/R_246);
R_EQ = R_3_246 + R1 + R5;

% Use the equivalent resistance to figure out the output current
I_OUT = V1 / R_EQ;

% Apply KCL and Ohm's Law to Figure out the rest
I_R5 = I_OUT;
I_R1 = I_R5;
V_R3 = V1 - I_R1*R1 - I_R5*R5;
I_R3 = V_R3 / R3;
I_R6 = V_R3 / R_246;
I_R2 = I_R6;
I_R4 = I_R6;

% Compute power from current and resistance 
P_R1 = R1*I_R1^2;
P_R2 = R2*I_R2^2;
P_R3 = R3*I_R3^2;
P_R4 = R4*I_R4^2;
P_R5 = R5*I_R5^2;
P_R6 = R6*I_R6^2;

% Print the current and power to 3 decimal points
fprintf("Current (Amps) and Power (Watts) for each resistor\n" + ...
    "(%.3f,%.3f)\n"+...
    "(%.3f,%.3f)\n"+...
    "(%.3f,%.3f)\n"+...
    "(%.3f,%.3f)\n"+...
    "(%.3f,%.3f)\n"+...
    "(%.3f,%.3f)\n", ...
    I_R1, P_R1, I_R2, P_R2, I_R3, P_R3, I_R4, P_R4, I_R5, P_R5, I_R6, P_R6);



