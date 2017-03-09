function [averageRatio, true_positive, false_positive, countMissclassifications] = train_and_test_scm_model(attempts, alpha, nposfiles, nnegfiles, sum_auto, min_auto, cross_corr_max)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
sumOfRatios = 0;
countMissclassifications = zeros(nposfiles+nnegfiles,1);
for i=1:attempts 
label = zeros(nposfiles+nnegfiles,1);
label(1:nposfiles,1) = 1;
trainIndex = randperm(nposfiles+nnegfiles,floor(alpha*nposfiles+nnegfiles)); 

index= linspace(1,nposfiles+nnegfiles,nposfiles+nnegfiles);
testIndex = setdiff(index,trainIndex);

% combine features
%trainObservations = [squeeze(auto_corr(:,3,trainIndex))' sum_auto(trainIndex,1) min_auto(trainIndex,1)];
trainObservations =  [sum_auto(trainIndex,:) min_auto(trainIndex,:)  cross_corr_max(:,trainIndex)' ];
trainLabels = label(trainIndex);
%testobservations = [squeeze(auto_corr(:,3,testIndex))' sum_auto(testIndex,1) min_auto(testIndex,1)];
testobservations = [sum_auto(testIndex,:) min_auto(testIndex,:) cross_corr_max(:,testIndex)' ];
testLabels = label(testIndex);
SVMModel = fitclinear(trainObservations,trainLabels);

[pred_labels,score] = predict(SVMModel,testobservations);

%pred_labels 
%testIndex
%score
predictions = (pred_labels == testLabels);
result = zeros(size(predictions,1),2);
result(:,1) = predictions;
result(:,2) = testIndex';

res = pred_labels - testLabels;
false_positive = length(res(res(:)==1))/nnegfiles;
true_positive = 1 - length(res(res(:)==-1))/nposfiles;

ratio = sum(predictions)/size(predictions,1);
sumOfRatios = sumOfRatios + ratio;
failingIndexes = testIndex(result(:,1) == 0);

countMissclassifications(failingIndexes,1) = countMissclassifications(failingIndexes,1) + 1;

end

averageRatio = sumOfRatios/attempts;
end

