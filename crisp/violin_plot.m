pkg load statistics

risp = csvread('./risp.csv')/60/60
crisp = csvread('./crisp.csv')/60/60

h = violin([risp,crisp], "smoothfactor", 50)
xlabel("Neuroprocessor")
ylabel("Training Time (Hours)")
set(gca, "xtick", 1:2, "xticklabel", {"RISP"; "CRISP"})
title("Training Time Results on Pavement Quality Dataset")
axis([-Inf, Inf, 0, 50])
set(gca, "xgrid", "on")


