function [ ] = plot_feature_clustering_1Dx3( vector1,vector2, vector3,nposfiles,nnegfiles)
    
    figure;
    subplot(3,1,1)
    nbr_bins_pos = 40;

    bin_size = (max(vector1(1:nposfiles))-min(vector1(1:nposfiles)))/ nbr_bins_pos;
    nbr_bins_neg = floor((max(vector1(nposfiles+1:nposfiles+nnegfiles))-min(vector1(nposfiles+1:nposfiles+nnegfiles)))/ bin_size)

    histogram(vector1(nposfiles+1:nposfiles+nnegfiles),nbr_bins_neg);
    hold on;
    histogram(vector1(1:nposfiles), nbr_bins_pos);
    title('sum in X dimension');
    legend('non-tampering','tampering');

    subplot(3,1,2);
    bin_size = (max(vector2(1:nposfiles))-min(vector2(1:nposfiles)))/ nbr_bins_pos;
    nbr_bins_neg = floor((max(vector2(nposfiles+1:nposfiles+nnegfiles))-min(vector2(nposfiles+1:nposfiles+nnegfiles)))/ bin_size)
    histogram(vector2(nposfiles+1:nposfiles+nnegfiles),nbr_bins_neg);
    hold on;
    histogram(vector2(1:nposfiles), nbr_bins_pos);
    title('Max in Y dimension');
    legend('non-tampering','tampering');
    
    subplot(3,1,3);
    bin_size = (max(vector3(1:nposfiles))-min(vector3(1:nposfiles)))/ nbr_bins_pos;
    nbr_bins_neg = floor((max(vector3(nposfiles+1:nposfiles+nnegfiles))-min(vector3(nposfiles+1:nposfiles+nnegfiles)))/ bin_size)
    histogram(vector3(nposfiles+1:nposfiles+nnegfiles),nbr_bins_neg);
    hold on;
    histogram(vector3(1:nposfiles), nbr_bins_pos);
    title('Max in Z dimension');
    legend('non-tampering','tampering');

end
