

I = imread('pedestrian.png');
fileID = fopen('pedestrian','w');
write_image_to_file(I,fileID);
fclose(fileID);



%% Write histogram to file
load('cell_features.mat');
fileID = fopen('histogram_parsed.txt','w');
formatSpec = '%d, ';
formatSpecIntro = 'cell [%d,%d] has histogram [';
formatSpecEnd = ']\n';
for row=1:16
    for col=1:8
        fprintf(fileID,formatSpecIntro, row,col);
        for value=1:9
            fprintf(fileID,formatSpec,cell_features(row,value,col));
        end
        fprintf(fileID,formatSpecEnd);
    end
    
end
fclose(fileID);

%% print image gradients to file

I = imread('pedestrian.png');
fileID = fopen('pedestrian_gradients.txt','w');
print_image_gradients(I,fileID);
fclose(fileID);

% %% 
% 
% 
% load('Beta');
% 
% function
% 
% nbrOfElements = size(features,1);
% 
% fileID = fopen('beta.txt','w');
% formatSpec = '%f\n';
% 
% for i=1:nbrOfElements
%     
%     fprintf(fileID,formatSpec,features(i,1));
% end
% 
% fclose(fileID);
% 
% %% sadasd
% 
% 
% 


