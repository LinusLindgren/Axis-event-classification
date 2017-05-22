function [ ] = plot_feature_clustering_1D( vector1, nposfiles,nnegfiles)
    %plot the histogram for positive and negative observations based on the
    %feature found in vector1
    figure;
    nbr_bins = 40;
    histogram(vector1(nposfiles+1:nposfiles+nnegfiles),nbr_bins);
    hold on;
    histogram(vector1(1:nposfiles), nbr_bins);
    title('Maximum value of raw data in the Y dimension');
    

end

