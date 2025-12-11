pkg load xlinprog

A = [
1 1 1 0
1 0 1 1
1 0 1 1
1 1 0 0
1 1 1 0
0 0 1 0
];

b = [10;11;11;5;10;5];

f = [1,1,1,1];

lb = [0,0,0,0,0,0];
ub=[inf,inf,inf,inf];

intcon = 1:4;
x0=[];
options = optimoptions('intlinprog', 'MaxTime', 100000);
[x,fval,exitflag,output]=intlinprog(f,intcon,[],[],A,b,lb,[]);
x
