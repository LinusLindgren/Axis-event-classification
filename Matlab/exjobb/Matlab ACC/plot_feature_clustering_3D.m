function [] = plot_feature_clustering_3D(vector1, vector2, vector3, nposfiles,nnegfiles)
%plot a 3 dimensional scatter plot based on the features found in vector1,
%vector2 and vector3
    figure;
    scatter3(vector1(1,nposfiles+1:(nposfiles+nnegfiles)), vector2(1,nposfiles+1:(nposfiles+nnegfiles)), vector3(1,nposfiles+1:(nposfiles+nnegfiles)));
    hold on;
    scatter3(vector1(1,1:nposfiles), vector2(1,1:nposfiles),vector3(1,1:nposfiles));
 
    
    title('Scatter block of the 3 features which lead to the biggest increase in accuracy');
    xlabel('Maximum value of raw data in the Y dimension');
    ylabel('Sum of autocorrelation of raw data in the Z dimension');
    zlabel('Maximum value of raw data in the X dimension');

end

