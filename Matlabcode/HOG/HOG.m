clear
clc


SVMModel = train_test_model(); %Training and testing set are specified in the function.
%% mine
c
mine_positive(SVMModel);

% 
% % Get list of all BMP files in this directory
% % DIR returns as a structure array.  You will need to use () and . to get
% % the file names.
% 
% posimagefiles = dir('exjobb/pos_MIT/*.ppm');      
% nposfiles = length(posimagefiles);    % Number of files found
% negimagefiles = dir('exjobb/neg_v3_mined/*.png');      
% nnegfiles = length(negimagefiles);    % Number of files found
% observations = zeros(nposfiles+nnegfiles, 3780);
% nbrfiles = nposfiles+nnegfiles;
% for ii=1:nposfiles
%    currentfilename = posimagefiles(ii).name;
%    currentimage = double(imread(currentfilename));
% 
%     histograms = extract_cell_features(currentimage);
%     observations(ii,:) = extract_features(histograms)
% end
% 
% for ii=1:nnegfiles
%    currentfilename = negimagefiles(ii).name;
%    currentimage = double(imread(currentfilename));
% 
%     histograms = extract_cell_features(currentimage);
%     observations(nposfiles+ii,:) = extract_features(histograms);
% end
% summa = 0;
% %for i= 1:1000
% %observations(isnan(observations))=0;
% alpha = 0.75;
% label = zeros(nbrfiles,1);
% label(1:nposfiles,1) = 1;
% trainIndex = randperm(nbrfiles,floor(alpha*nbrfiles)); 
% index= linspace(1,nbrfiles,nbrfiles);
% testIndex = setdiff(index,trainIndex);
% trainObservations = observations(trainIndex,:);
% trainLabels = label(trainIndex);
% testobservations = observations(testIndex,:);
% testLabels = label(testIndex);
% SVMModel = fitclinear(trainObservations,trainLabels);
% %SVMModel = fitcsvm(trainObservations,trainLabels,'KernelFunction','rbf','Standardize',true,'ClassNames',[0,1]);
% 
% [pred_labels,score] = predict(SVMModel,testobservations);
% 
% %pred_labels 
% %testIndex
% %score
% predictions = (pred_labels == testLabels);
% %testLabels-pred_labels
% ratio = sum(predictions)/size(predictions,1)

%% Try data mining

% clc;
% imagefiles = dir('exjobb/data_mining_images/*.png');      
% nbrimagefiles = length(imagefiles);    % Number of files found
% 
% for ii=1:nbrimagefiles
%     currentimage = imread(imagefiles(ii).name);
%     workingimage = double(currentimage);
%     %imagesc(currentimage);
%     positives = parse_image(workingimage,SVMModel);
%     for i=1:size(positives,1)
%         %disp('before write');
%         fileNoExt = imagefiles(ii).name;
%         fileNoExt = fileNoExt(1:(length(fileNoExt)-4));
%         filename = char(['exjobb/data_mining_images_result/' fileNoExt int2str(i)]); 
%         imwrite(currentimage(positives(i,1):(positives(i,1)+127), positives(i,2):(positives(i,2)+63),1:3),char([filename '.png']));
%         %disp('after write');
%     end
% end




%summa = summa + ratio;
%end
%successrate = summa/1000








    
    