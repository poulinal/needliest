%%% setup initial parameters
% 
initial_pressure = 101325;       % in Pa (1 atm)
initial_temperature = 300;       % in Kelvin
valve_opening_fraction = 0.7;    % between 0 and 1
pipe_diameter = 0.01;            % in meters
flow_resistance = 1e5;           % in Pa·s/m³ or appropriate units


t = [0, 5];
t_static = [0];


tarPSI = 100; % target resistance (in psi)
% tarPa = 
t_tarPSI = [t,tarPSI]; % target resistance (in psi) but time matrix

prevRest = 0; % previous resistance (0 is closed, 1 is open)
t_prevRest = [t,prevRest];

test_resi = [0, 1];
t_test_resi = test_resi;


% k_i = 0.000005;
% k_p = 300;
% k_d = 10;
k_i = 0;
k_p = 0.000000000005;
k_d = 0;
% SP = 100; %setpoint / target value
SP = 689475;
Last_error = 0;
Last_iterm = 0;

t_k_i = [t_static, k_i];
t_k_p = [t_static, k_p];
t_k_d = [t_static, k_d];
t_SP = [t_static, SP]; %setpoint / target value
t_Last_error = [t_static, Last_error];
t_Last_iterm = [t_static, Last_iterm];

