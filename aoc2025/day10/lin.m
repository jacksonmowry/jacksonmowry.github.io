pkg load xlinprog

A = [
0 0 0 0 1 1;
0 1 0 0 0 1;
0 0 1 1 1 0;
1 1 0 1 0 0
];

b = [3;5;4;7];

f = [1,1,1,1,1,1];

lb = [0,0,0,0,0,0];
ub=[inf,inf,inf,inf];

Aeq=[];
beq=[];
intcon = 1:6;
x0=[];
options = optimoptions('intlinprog','Display', 'off', 'MaxTime', 100000);
[x,fval,exitflag,output]=intlinprog(f,intcon,[],[],A,b,lb,[]);

