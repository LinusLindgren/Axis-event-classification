%[x,y,z] = textread (octave_test, "%f %f %f")
k = 1:256;

figure (1);
plot (k, x);
xlabel ("x");
ylabel ("ACC(X)");
figure (2);
plot (k, y);
xlabel ("x");
ylabel ("ACC(Y)");
figure (3);
plot (k, z);
xlabel ("x");
ylabel ("ACC(Z)");

%plot (k, x);

title ("PLOT");
67867