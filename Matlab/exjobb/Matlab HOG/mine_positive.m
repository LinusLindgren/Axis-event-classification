function [ ] = mine_positive( SVMModel )
%MINE_POSITIVE Summary of this function goes here
%   Detailed explanation goes here
imagefiles = dir('exjobb/data_mining_images/*.png');      
nbrimagefiles = length(imagefiles);    % Number of files found
figure;
for ii=1:nbrimagefiles
    currentimage = imread(imagefiles(ii).name);
    workingimage = double(currentimage);
    %imagesc(currentimage);
    tic
    positives = parse_image(workingimage,SVMModel);
    toc
    imagesc(currentimage);
    hold on;
    
    
    for i=1:size(positives,1)
        rectangle('Position',[positives(i,2) positives(i,1) 64 128], 'LineWidth',2, 'EdgeColor','g');
        hold on;
        %disp('before write');
        fileNoExt = imagefiles(ii).name;
        fileNoExt = fileNoExt(1:(length(fileNoExt)-4));
        filename = char(['exjobb/data_mining_images_result/' fileNoExt int2str(i)]); 
        imwrite(currentimage(positives(i,1):(positives(i,1)+127), positives(i,2):(positives(i,2)+63),1:3),char([filename '.png']));
        %disp('after write');
    end
end

end

