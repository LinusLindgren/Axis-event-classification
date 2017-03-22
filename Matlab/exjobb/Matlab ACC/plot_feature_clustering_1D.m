function [ ] = plot_feature_clustering_1D( vector1, nposfiles,nnegfiles)
    

    figure;
    nbr_bins = 40;
    histogram(vector1(nposfiles+1:nposfiles+nnegfiles),nbr_bins);
    hold on;
    histogram(vector1(1:nposfiles), nbr_bins);
    title('Maximum value of raw data in the Y dimension');
    

end

