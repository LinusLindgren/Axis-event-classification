function [averageTestRatio, averageTrainRatio, true_positive_ratio, false_positive_ratio, countMissclassifications,SVMModel] = train_and_test_scm_model(attempts, alpha, nposfiles, nnegfiles, sum_auto, min_auto, cross_corr_max, auto_corr_flat, auto_bins, ...
    meanFeatures, meanTiltFeatures, stdFeatures, stdTiltFeatures,sumFeatures,minFeatures,maxFeatures,maxTiltFeatures,minTiltFeatures)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
sumOfRatios_test = 0;
sumOfRatios_train = 0;
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
%trainObservations =  [sum_auto(trainIndex,:) min_auto(trainIndex,:)  cross_corr_max(2,trainIndex)' ];
%ONES = rand(size(cross_corr_max));
trainObservations =  [sum_auto(trainIndex,:) min_auto(trainIndex,:)  cross_corr_max(2,trainIndex)' meanFeatures(:,trainIndex)' ...
    stdFeatures(:,trainIndex)' sumFeatures(:,trainIndex)' minFeatures(:,trainIndex)' ...
maxFeatures(:,trainIndex)'];
trainLabels = label(trainIndex);
%testobservations = [squeeze(auto_corr(:,3,testIndex))' sum_auto(testIndex,1) min_auto(testIndex,1)];
testobservations = [sum_auto(testIndex,:) min_auto(testIndex,:) cross_corr_max(2,testIndex)' meanFeatures(:,testIndex)' ... 
    stdFeatures(:,testIndex)' sumFeatures(:,testIndex)' minFeatures(:,testIndex)' maxFeatures(:,testIndex)' ];

mean_train = mean(trainObservations);
std_train = std(trainObservations);
%mean_train(:,1:3) = mean_train(:,1:3)*2;
%trainObservations = trainObservations - repmat(mean_train,size(trainObservations,1),1);
%testobservations = testobservations - repmat(mean_train,size(testobservations,1),1);
%trainObservations = trainObservations ./ repmat(std_train,size(trainObservations,1),1);
%testobservations = testobservations ./ repmat(std_train,size(testobservations,1),1);

trainObservations= trainObservations - repmat(mean_train,size(trainObservations,1),1);
testobservations = testobservations - repmat(mean_train,size(testobservations,1),1);
trainObservations = trainObservations ./ repmat(std_train,size(trainObservations,1),1);
testobservations = testobservations ./ repmat(std_train,size(testobservations,1),1);

%trainObservations(:,1:3) = trainObservations(:,1:3) - repmat(mean_train(1,1:3),size(trainObservations,1),1);
%testobservations(:,1:3) = testobservations(:,1:3) - repmat(mean_train(1,1:3),size(testobservations,1),1);
%trainObservations(:,1:3) = trainObservations(:,1:3) ./ repmat(std_train(1,1:3),size(trainObservations,1),1);
%testobservations(:,1:3) = testobservations(:,1:3) ./ repmat(std_train(1,1:3),size(testobservations,1),1);

testLabels = label(testIndex);
SVMModel = fitclinear(trainObservations,trainLabels);
%Try and predict values with the calculated SVMModel for both the train and
%test data.
[pred_labels_train,~] = predict(SVMModel,trainObservations);
[pred_labels_test,~] = predict(SVMModel,testobservations);

predictions_train = (pred_labels_train == trainLabels);
predictions_test = (pred_labels_test == testLabels);

%Used for counting misclassifications on the individual samples.
result = zeros(size(predictions_test,1),2);
result(:,1) = predictions_test;
result(:,2) = testIndex';

res_test = pred_labels_test - testLabels;
false_positive = length(res_test(res_test(:)==1))/(size(testLabels,1)-sum(testLabels));
true_positive = 1 - length(res_test(res_test(:)==-1))/sum(testLabels);
false_positive_sum = false_positive_sum + false_positive;
true_positive_sum = true_positive_sum + true_positive;

ratio_train = sum(predictions_train)/size(predictions_train,1);
sumOfRatios_train = sumOfRatios_train + ratio_train;
ratio_test = sum(predictions_test)/size(predictions_test,1);
sumOfRatios_test = sumOfRatios_test + ratio_test;

failingIndexes = testIndex(result(:,1) == 0);

countMissclassifications(failingIndexes,1) = countMissclassifications(failingIndexes,1) + 1;

end

averageTestRatio = sumOfRatios_test/attempts;
averageTrainRatio = sumOfRatios_train/attempts;
true_positive_ratio = true_positive_sum/attempts;
false_positive_ratio = false_positive_sum/attempts; 
end

