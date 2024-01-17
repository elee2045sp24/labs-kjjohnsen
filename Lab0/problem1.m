V_in = 5; %Input voltage in volts
R1 = 10; %Resistance in ohms
R2 = 5;

I_in = V_in / (R1 + R2); %by ohm's law

P_R1 = I_in^2*R1;
P_R2 = I_in^2*R2;
V_out = I_in*R2;

fprintf("V out = %f volts, Power (R1,R2) = %f,%f watts\n", ...
    V_out, P_R1, P_R2);

