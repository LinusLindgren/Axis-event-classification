function [ ] = plot_feature_clustering( vector1, vector2, nposfiles,nnegfiles)
    figure;
    scatter(vector1(1,1:nposfiles), vector2(1,1:nposfiles), 10, 'r');
    hold on;
    scatter(vector1(1,nposfiles+1:(nposfiles+nnegfiles)), vector2(1,nposfiles+1:(nposfiles+nnegfiles)), 10, 'b');
    title('Scatter block of 2 maximum weighted features');
    xlabel('Sum of the derivatives of the raw data on the Y-dimension');
    ylabel('Minimum of auto-correlation on the Z-dimension');

end

