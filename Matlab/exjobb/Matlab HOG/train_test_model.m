function [ SVMModel] = train_test_model()

%TRAIN_MODEL Summary of this function goes here
%   Detailed explanation goes here



% Get list of all BMP files in this directory
% DIR returns as a structure array.  You will need to use () and . to get
% the file names.



posimagefiles = dir('exjobb/pos_MIT/*.ppm');      
nposfiles = length(posimagefiles);    % Number of files found
negimagefiles = dir('exjobb/neg_v3_mined/*.png');      
nnegfiles = length(negimagefiles);    % Number of files found
observations = zeros(nposfiles+nnegfiles, 3780);
nbrfiles = nposfiles+nnegfiles;

for ii=1:nposfiles
   currentfilename = posimagefiles(ii).name;
   currentimage = double(imread(currentfilename));

    histograms = extract_cell_features(currentimage);
    observations(ii,:) = extract_features(histograms);

end



for ii=1:nnegfiles

   currentfilename = negimagefiles(ii).name;
   currentimage = double(imread(currentfilename));



    %histograms = extract_cell_features(currentimage);

    histograms = calculate_all_cell_histograms(currentimage);
    observations(nposfiles+ii,:) = extract_window_features(histograms,1,1);

    %observations(nposfiles+ii,:) = extract_features(histograms);

end

summa = 0;
%for i= 1:1000
%observations(isnan(observations))=0;
alpha = 0.75;
label = zeros(nbrfiles,1);
label(1:nposfiles,1) = 1;
trainIndex = randperm(nbrfiles,floor(alpha*nbrfiles)); 
index= linspace(1,nbrfiles,nbrfiles);
testIndex = setdiff(index,trainIndex);
trainObservations = observations(trainIndex,:);
trainLabels = label(trainIndex);
testobservations = observations(testIndex,:);
testLabels = label(testIndex);
SVMModel = fitclinear(trainObservations,trainLabels);

%SVMModel = fitcsvm(trainObservations,trainLabels,'KernelFunction','rbf','Standardize',true,'ClassNames',[0,1]);

[pred_labels,score] = predict(SVMModel,testobservations);



%pred_labels 
%testIndex
%score
predictions = (pred_labels == testLabels);

res = pred_labels - testLabels;
disp('false positve');
length(res(res(:)==1))
disp('false negative');
length(res(res(:)==-1))
disp('ratio of false positve');
length(res(res(:)==1)) / (length(res(res(:)==1))+length(res(res(:)==-1)))
%testLabels-pred_labels
ratio = sum(predictions)/size(predictions,1)



end
