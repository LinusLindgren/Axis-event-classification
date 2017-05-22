function [] = plot_samples( samples,nposfiles,nnegfiles )
%plot the raw samples of one positive and one negative observation in the
%same plot. One plot for each dimension
random_pos_file = randi([1 nposfiles],1,1);
random_neg_file = randi([nposfiles+1 nnegfiles],1,1);
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

