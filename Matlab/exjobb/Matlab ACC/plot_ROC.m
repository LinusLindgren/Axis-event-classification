function [] = plot_ROC( true_positive, false_positive,color, plot_title)
%PLOT_ROC - create ROC plot consisting of the dots found in true_positive
%and false_positive
limits = [0 1 0 1]
figure;
scatter(false_positive,true_positive,'filled');

title(plot_title);
axis(limits);
text(false_positive + 0.002,true_positive,['DT    '; 'KNN   '; 'SVM-L '; 'SVM-NL'],'HorizontalAlignment','right')


end

