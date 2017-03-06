function [] = plot_samples( samples,nposfiles,nnegfiles )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

random_pos_file = floor(rand(1)*nposfiles);
random_neg_file = floor(rand(1)*nnegfiles)+nposfiles;
%Plot X,Y,Z Axis for one neg and one pos file seperatly
figure;
title('X-axis random positive(green) and random negative(red) data');
plot(1:size(samples,1),samples(:,1,random_pos_file), 'g');
hold on;
plot(1:size(samples,1),samples(:,1,random_neg_file), 'r');

figure;
title('Y-axis random positive(green) and random negative(red) data');
plot(1:size(samples,1),samples(:,2,random_pos_file), 'g');
hold on;
plot(1:size(samples,1),samples(:,2,random_neg_file), 'r');

figure;
title('Z-axis random positive(green) and random negative(red) data');
plot(1:size(samples,1),samples(:,3,random_pos_file), 'g');
hold on;
plot(1:size(samples,1),samples(:,3,random_neg_file), 'r');




end

