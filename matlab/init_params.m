%%% setup initial parameters

% set basic time sequence
t = [0, 5];
t_static = [0];

%intial PID parameters
k_i = 0.00000000003;
% k_i = 0.0000000000003;
% k_p = 0.000000000024;
k_p = 0.000000000012;
k_d = 0.000000000002;%0.000000000005;

%SP is set point
% SP = 689475;% 6.89 * 10^5 % 100psi
SP = 4136854; %4,136,854.368 % 600psi

%set zeroed
Last_error = 0;
Last_iterm = 0;

%set parameters in time sequence
t_k_i = [t_static, k_i];
t_k_p = [t_static, k_p];
t_k_d = [t_static, k_d];
t_SP = [t_static, SP]; %setpoint / target value
t_Last_error = [t_static, Last_error];
t_Last_iterm = [t_static, Last_iterm];

