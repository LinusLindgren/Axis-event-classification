function [] = plot_feature_clustering_3D(vector1, vector2, vector3, nposfiles,nnegfiles)

    figure;
    scatter3(vector1(1,nposfiles+1:(nposfiles+nnegfiles)), vector2(1,nposfiles+1:(nposfiles+nnegfiles)), vector3(1,nposfiles+1:(nposfiles+nnegfiles)));
    hold on;
    scatter3(vector1(1,1:nposfiles), vector2(1,1:nposfiles),vector3(1,1:nposfiles));
 
    
    title('Scatter block of 3 maximum weighted features');
    xlabel('Maximum value of raw data in the Y dimension');
    ylabel('Minimum value of raw data in the Y dimension');
    zlabel('Sum of the discrete derivative in the Y dimension');

end

