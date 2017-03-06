clear, clc, close all
[nposfiles,nnegfiles,samples] = parse_acc_files();

plot_samples(samples,nposfiles,nnegfiles);


