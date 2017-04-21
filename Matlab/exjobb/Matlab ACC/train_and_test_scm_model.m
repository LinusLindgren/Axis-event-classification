function [averageTestRatio, averageTrainRatio, true_positive_ratio, false_positive_ratio, countMissclassifications,SVMModel, ... 
    featureVector, scores_positive_train_total, scores_negative_train_total, scores_positive_test_total, scores_negative_test_total, ... 
    mean_train, std_train, SVMModels] = train_and_test_scm_model(attempts, alpha, nposfiles, nnegfiles, sum_auto, min_auto, cross_corr_max, ...
    auto_corr_flat, auto_bins, meanFeatures, meanTiltFeatures, stdFeatures, stdTiltFeatures,sumFeatures,minFeatures,maxFeatures, ... 
    maxTiltFeatures,minTiltFeatures, skewness_vec, kurtosis_vec, sum_changes, mean_changes, ...
    der_min, der_max, der_mean, der_sum, sumAbsFeatures,sumAllDimFeatures, moments, skewness_acor, kurtosis_acor, ...
    sum_changes_auto, mean_changes_auto, der_min_auto_corr, der_max_auto_corr, der_mean_auto_corr, der_sum_auto_corr,write_svm_model_to_file, ...
    psdx_nbrPeaks, psdx_nbrPeaks_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, ...
    skewness_psdx, skewness_tilt_psdx ,kurtosis_psdx, kurtosis_tilt_psdx, index_of_first_max, index_of_first_min,max_changes, max_changes_index,max_changes_auto, max_changes_index_auto)

%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
sumOfRatios_test = 0;
sumOfRatios_train = 0;
false_positive_sum = 0;
true_positive_sum = 0;
countMissclassifications = zeros(nposfiles+nnegfiles,1);

scores_positive_train_total = [];
scores_negative_train_total = [];
scores_positive_test_total = [];
scores_negative_test_total = [];


scores_positive_test_total = [];
scores_negative_test_total = [];
for i=1:attempts 
label = zeros(nposfiles+nnegfiles,1);
label(1:nposfiles,1) = 1;
trainIndex = randperm(nposfiles+nnegfiles,floor(alpha*nposfiles+nnegfiles)); 

index= linspace(1,nposfiles+nnegfiles,nposfiles+nnegfiles);
testIndex = setdiff(index,trainIndex);

% combine features
%trainObservations =  [sum_auto(trainIndex,:) min_auto(trainIndex,:)  cross_corr_max(2,trainIndex)' ];
%ONES = rand(size(cross_corr_max));
%Kurotisis_acor i z-led 0.0016
%

% featureVector = [sum_auto(:,3) min_auto(:,2:3) cross_corr_max(2,:)' meanFeatures' ...
%  minFeatures' ...
% maxFeatures(1:2,:)'  skewness_vec(:,2:3) kurtosis_acor() skewness_acor ...
% sum_changes'  squeeze(psdx_peak_freq_bin_tilt(:,1,:))' squeeze(psdx_peak_freq_bin_tilt(:,3,:))'...
% skewness_psdx psdx_nbrPeaks_tilt' ...
% squeeze(psdx_peak_freq_bin(:,1,:))' squeeze(psdx_peak_freq_bin(:,2,:))' index_of_first_max(3,:)' meanTiltFeatures(2,:)'  stdFeatures(2,:)' sum_changes_auto'];

%BEST FOR 1.28 sek 200 HZ
featureVector = [sum_auto(:,3) min_auto(:,2:3) meanFeatures([1],:)' ...
 minFeatures(2:3,:)' ...
maxFeatures(1:2,:)'  skewness_vec(:,2:3) kurtosis_acor(:,[1 3]) skewness_acor ...
sum_changes'  squeeze(psdx_peak_freq_bin_tilt([1],1,:)) squeeze(psdx_peak_freq_bin_tilt(:,3,:))'...
skewness_psdx psdx_nbrPeaks_tilt' ...
squeeze(psdx_peak_freq_bin([1 3 4 5],1,:))' squeeze(psdx_peak_freq_bin([1 5],2,:))' ...
index_of_first_max(3,:)' meanTiltFeatures(2,:)'  stdFeatures(2,:)' ...
sum_changes_auto(1:2,:)' ];

%resultasd = test_features(featureVector,100, alpha, nposfiles, nnegfiles);
%Markers
%psdx_nbrPeals_tilt(3) = 16
%psdx_peak_freq_bin_tilt(1,3,:) = 26; 
% squeeze(psdx_peak_freq_bin_tilt(2,2,:)) = 29
% mean_features = mean(featureVector);
% std_features = std(featureVector);
% featureVector= featureVector - repmat(mean_features,size(featureVector,1),1);
% featureVector = featureVector ./ repmat(std_features,size(featureVector,1),1);

% trainObservations =  [sum_auto(trainIndex,:) min_auto(trainIndex,:)  cross_corr_max(2,trainIndex)' meanFeatures(:,trainIndex)' ...
%  minFeatures(:,trainIndex)' ...
% maxFeatures(:,trainIndex)'  skewness_vec(trainIndex,:) kurtosis_acor(trainIndex,:) skewness_acor(trainIndex,:) ...
% sum_changes(:,trainIndex)'  squeeze(psdx_peak_freq_bin_tilt(:,1,trainIndex))' squeeze(psdx_peak_freq_bin_tilt(:,2,trainIndex))' squeeze(psdx_peak_freq_bin_tilt(:,3,trainIndex))'...
% skewness_psdx(trainIndex,:) psdx_nbrPeaks_tilt(:,trainIndex)' ...
% squeeze(psdx_peak_freq_bin(:,1,trainIndex))' squeeze(psdx_peak_freq_bin(:,2,trainIndex))' ];

trainObservations = featureVector(trainIndex,:);

%psdx_nbrPeaks(:,trainIndex)' psdx_nbrPeaks_tilt(:,trainIndex)'
%trainObservations = [ sum_changes(2,trainIndex)'  min_auto(trainIndex,3)];
%trainObservations = ONES(2,trainIndex)';
trainLabels = label(trainIndex);
%testobservations = ONES(2,testIndex)';
% testobservations = [sum_auto(testIndex,:) min_auto(testIndex,:) cross_corr_max(2,testIndex)' meanFeatures(:,testIndex)' ... 
%  minFeatures(:,testIndex)' maxFeatures(:,testIndex)'  skewness_vec(testIndex,:) kurtosis_acor(testIndex,:) skewness_acor(testIndex,:) ...
%  sum_changes(:,testIndex)' squeeze(psdx_peak_freq_bin_tilt(:,1,testIndex))' squeeze(psdx_peak_freq_bin_tilt(:,2,testIndex))' squeeze(psdx_peak_freq_bin_tilt(:,3,testIndex))'...
% skewness_psdx(testIndex,:) psdx_nbrPeaks_tilt(:,testIndex)' ...
% squeeze(psdx_peak_freq_bin(:,1,testIndex))' squeeze(psdx_peak_freq_bin(:,2,testIndex))' ];
testobservations = featureVector(testIndex,:);
%testobservations = [ sum_changes(2,testIndex)'  min_auto(testIndex,3)];
mean_train = mean(trainObservations);
std_train = std(trainObservations);
%mean_train(:,1:3) = mean_train(:,1:3)*2;
%trainObservations = trainObservations - repmat(mean_train,size(trainObservations,1),1);
%testobservations = testobservations - repmat(mean_train,size(testobservations,1),1);
%trainObservations = trainObservations ./ repmat(std_train,size(trainObservations,1),1);
%testobservations = testobservations ./ repmat(std_train,size(testobservations,1),1);
% 
trainObservations= trainObservations - repmat(mean_train,size(trainObservations,1),1);
testobservations = testobservations - repmat(mean_train,size(testobservations,1),1);
trainObservations = trainObservations ./ repmat(std_train,size(trainObservations,1),1);
testobservations = testobservations ./ repmat(std_train,size(testobservations,1),1);
featureVector= featureVector - repmat(mean_train,size(featureVector,1),1);
featureVector = featureVector ./ repmat(std_train,size(featureVector,1),1);


%trainObservations(:,1:3) = trainObservations(:,1:3) - repmat(mean_train(1,1:3),size(trainObservations,1),1);
%testobservations(:,1:3) = testobservations(:,1:3) - repmat(mean_train(1,1:3),size(testobservations,1),1);
%trainObservations(:,1:3) = trainObservations(:,1:3) ./ repmat(std_train(1,1:3),size(trainObservations,1),1);
%testobservations(:,1:3) = testobservations(:,1:3) ./ repmat(std_train(1,1:3),size(testobservations,1),1);

testLabels = label(testIndex);
%Above model is a K-Nearest neighbour, temporary name is nice
%SVMModel = fitctree(trainObservations,trainLabels);
%SVMModel = TreeBagger(10,trainObservations,trainLabels)
%SVMModel = fitcknn(trainObservations,trainLabels,'NumNeighbors',15,'Standardize',1);

weights = zeros(nposfiles+nnegfiles,1);
weights(1:nposfiles,1) = 1 / nposfiles;
weights(nposfiles+1:nposfiles+nnegfiles,1) = 1 / nnegfiles;
% Cost = [0 (nposfiles/nnegfiles); 1  0];
% SVMModel = fitcknn(trainObservations,trainLabels,'NumNeighbors',15,'Standardize',1, 'Cost', Cost);
%SVMModel = fitctree(trainObservations,trainLabels);
SVMModel = fitclinear(trainObservations,trainLabels);
SVMModels{i} = SVMModel;
%result = test_features( featureVector , attempts, alpha, nposfiles, nnegfiles);
%, 'weights', weights(trainIndex
%Try and predict values with the calculated SVMModel for both the train and
%test data.

[pred_labels_train,score_train] = predict(SVMModel,trainObservations);
[pred_labels_test,score_test] = predict(SVMModel,testobservations);


predictions_train = (pred_labels_train == trainLabels);
predictions_test = (pred_labels_test == testLabels);

[pred_labels_feature_vector,scores] = predict(SVMModel,featureVector);
score_positives = scores(1:nposfiles,2);
score_negatives = scores(nposfiles+1:nposfiles+nnegfiles,2);

pos_train_scores = scores(trainIndex(trainIndex <= nposfiles),2);    
neg_train_scores = scores(trainIndex(trainIndex > nposfiles),2);    
pos_test_scores = scores(testIndex(testIndex <= nposfiles),2);    
neg_test_scores = scores(testIndex(testIndex > nposfiles),2); 

scores_positive_train_total = [scores_positive_train_total ; pos_train_scores] ;
scores_negative_train_total = [scores_negative_train_total ; neg_train_scores];
scores_positive_test_total = [scores_positive_test_total ; pos_test_scores] ;
scores_negative_test_total = [scores_negative_test_total ; neg_test_scores];



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
% scores_positive_train_total = [scores_positive_train_total, scores_positive_train];
% scores_negative_train_total = [scores_negative_train_total, scores_negative_train];
% scores_positive_test_total = [scores_positive_test_total, scores_positive_test];
% scores_negative_test_total = [scores_negative_test_total, scores_negative_test];
end
if(write_svm_model_to_file)
    write_svm_model(SVMModel,mean_train, std_train);
end

averageTestRatio = sumOfRatios_test/attempts;
averageTrainRatio = sumOfRatios_train/attempts;
true_positive_ratio = true_positive_sum/attempts;
false_positive_ratio = false_positive_sum/attempts; 
end

