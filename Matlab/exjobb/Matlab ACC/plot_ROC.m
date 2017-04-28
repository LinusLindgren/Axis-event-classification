function [] = plot_ROC( true_positive, false_positive,color, plot_title)
%PLOT_ROC Summary of this function goes here
%   Detailed explanation goes here
limits = [0 1 0 1]
figure;
%plot(false_positive,true_positive, color);
scatter(false_positive,true_positive,'filled');

title(plot_title);
axis(limits);
text(false_positive + 0.002,true_positive,['DT    '; 'KNN   '; 'SVM-L '; 'SVM-NL'],'HorizontalAlignment','right')


end

