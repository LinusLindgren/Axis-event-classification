function [ ] = plot_feature_clustering_2D( vector1, vector2, nposfiles,nnegfiles)
    figure;
    scatter(vector1(1,1:nposfiles), vector2(1,1:nposfiles), 10, 'r');
    hold on;
    scatter(vector1(1,nposfiles+1:(nposfiles+nnegfiles)), vector2(1,nposfiles+1:(nposfiles+nnegfiles)), 10, 'b');
    title('Scatter block of 2 maximum weighted features');
    xlabel('Maximum value of raw data in the Y dimension');
    ylabel('Minimum value of raw data in the Y dimension');

end

