
% RK4: 4th order Runge-Kutta method
function y = RK4(fun,y,t,h)

k1 = fun(t,y);
k2 = fun(t+1/2*h,y+1/2*h*k1);
k3 = fun(t+1/2*h,y+1/2*h*k2);
k4 = fun(t+h,y+h*k3);

y = y + 1/6*h*(k1 + 2*k2 + 2*k3 + k4);
t = t + h;
end