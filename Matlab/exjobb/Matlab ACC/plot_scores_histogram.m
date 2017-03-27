function [] = plot_scores_histogram(scores_positive_train, scores_negative_train, scores_positive_test, scores_negative_test)
%PLOT_SCORES_HISTOGRAM Summary of this function goes here
%   Detailed explanation goes here

positive_span_train = max(scores_positive_train)-min(scores_positive_train);
negative_span_train = max(scores_negative_train)-min(scores_negative_train);

positive_span_test = max(scores_positive_test)-min(scores_positive_test);
negative_span_test = max(scores_negative_test)-min(scores_negative_test);

%Find out which span is wider to accomodate the other span
if(positive_span_train > negative_span_train)
    vector1_train = scores_positive_train;
    vector2_train = scores_negative_train;
else
    vector1_train = scores_negative_train;
    vector2_train = scores_positive_train;
end
nbr_bins = 200;

%Calculate bin size so that we can calculate the amount of bins in vector2,
%should be a less amount of bins
bin_size_train = (max(vector1_train)-min(vector1_train))/ nbr_bins;
nbr_bins_vector2_train = ceil(((max(vector2_train)-min(vector2_train))/ bin_size_train));

%Do the same for test
if(positive_span_test > negative_span_test)
    vector1_test = scores_positive_test;
    vector2_test = scores_negative_test;
else
    vector1_test = scores_negative_test;
    vector2_test = scores_positive_test;
end

bin_size_test = (max(vector1_test)-min(vector1_test))/ nbr_bins;
nbr_bins_vector2_test = ceil(((max(vector2_test)-min(vector2_test))/ bin_size_test));

%If positive wide span is bigger then it is vector1, if not it is vector2.
%Negative should be printed first so that it gets the color blue.
if(positive_span_train > negative_span_train)
    plot_histos(vector1_train,vector2_train,nbr_bins,nbr_bins_vector2_train);
else
    plot_histos(vector2_train,vector1_train,nbr_bins_vector2_train,nbr_bins);
end

if(positive_span_test > negative_span_test)
    plot_histos(vector1_test,vector2_test,nbr_bins,nbr_bins_vector2_test);
else
    plot_histos(vector2_test,vector1_test,nbr_bins_vector2_test,nbr_bins);
end

end

function [] = plot_histos(vector1, vector2, nbr_bins1, nbr_bins2)
    
    figure;
    histogram(vector2,nbr_bins2);
    hold on;
    histogram(vector1,nbr_bins1);

end

