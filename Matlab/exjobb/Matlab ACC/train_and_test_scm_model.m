function [averageRatio, true_positive_ratio, false_positive_ratio, countMissclassifications,SVMModel] = train_and_test_scm_model(attempts, alpha, nposfiles, nnegfiles, sum_auto, min_auto, cross_corr_max)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
sumOfRatios = 0;
false_positive_sum = 0;
true_positive_sum = 0;
countMissclassifications = zeros(nposfiles+nnegfiles,1);
for i=1:attempts 
label = zeros(nposfiles+nnegfiles,1);
label(1:nposfiles,1) = 1;
trainIndex = randperm(nposfiles+nnegfiles,floor(alpha*nposfiles+nnegfiles)); 

index= linspace(1,nposfiles+nnegfiles,nposfiles+nnegfiles);
testIndex = setdiff(index,trainIndex);

% combine features
%trainObservations = [squeeze(auto_corr(:,3,trainIndex))' sum_auto(trainIndex,1) min_auto(trainIndex,1)];
trainObservations =  [sum_auto(trainIndex,:) min_auto(trainIndex,:)  cross_corr_max(2,trainIndex)' ];
trainLabels = label(trainIndex);
%testobservations = [squeeze(auto_corr(:,3,testIndex))' sum_auto(testIndex,1) min_auto(testIndex,1)];
testobservations = [sum_auto(testIndex,:) min_auto(testIndex,:) cross_corr_max(2,testIndex)' ];

mean_train = mean(trainObservations);
std_train = std(trainObservations);
%mean_train(:,1:3) = mean_train(:,1:3)*2;
%trainObservations = trainObservations - repmat(mean_train,size(trainObservations,1),1);
%testobservations = testobservations - repmat(mean_train,size(testobservations,1),1);
%trainObservations = trainObservations ./ repmat(std_train,size(trainObservations,1),1);
%testobservations = testobservations ./ repmat(std_train,size(testobservations,1),1);

trainObservations(:,7) = trainObservations(:,7) - repmat(mean_train(1,7),size(trainObservations,1),1);
testobservations(:,7) = testobservations(:,7) - repmat(mean_train(1,7),size(testobservations,1),1);
trainObservations(:,7) = trainObservations(:,7) ./ repmat(std_train(1,7),size(trainObservations,1),1);
testobservations(:,7) = testobservations(:,7) ./ repmat(std_train(1,7),size(testobservations,1),1);

%trainObservations(:,1:3) = trainObservations(:,1:3) - repmat(mean_train(1,1:3),size(trainObservations,1),1);
%testobservations(:,1:3) = testobservations(:,1:3) - repmat(mean_train(1,1:3),size(testobservations,1),1);
%trainObservations(:,1:3) = trainObservations(:,1:3) ./ repmat(std_train(1,1:3),size(trainObservations,1),1);
%testobservations(:,1:3) = testobservations(:,1:3) ./ repmat(std_train(1,1:3),size(testobservations,1),1);

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
false_positive = length(res(res(:)==1))/(size(testLabels,1)-sum(testLabels));
true_positive = 1 - length(res(res(:)==-1))/sum(testLabels);
false_positive_sum = false_positive_sum + false_positive;
true_positive_sum = true_positive_sum + true_positive;
ratio = sum(predictions)/size(predictions,1);
sumOfRatios = sumOfRatios + ratio;
failingIndexes = testIndex(result(:,1) == 0);

countMissclassifications(failingIndexes,1) = countMissclassifications(failingIndexes,1) + 1;

end

averageRatio = sumOfRatios/attempts;
true_positive_ratio = true_positive_sum/attempts;
false_positive_ratio = false_positive_sum/attempts; 
end

