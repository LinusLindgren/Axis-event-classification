%% read samples
clear, clc, close all
sampling_time = 1.28;
target_freq = 200;
 
%read samples with sampling frequency 200 Hz
[nposfiles1,nnegfiles1,samples1] = parse_acc_files(200*sampling_time,'acc_data\freq200temp\postemp\acc*' ...
 , 'acc_data\freq200temp\negtemp\acc*');
 [samples1, ~] = convert_freq(samples1,200,target_freq);
 
 %read samples with sampling frequency 499 Hz from door 1-13
[nposfiles2,nnegfiles2,samples2] = read_samples(400*sampling_time,1,13);
[samples2, ~] = convert_freq(samples2,400,target_freq);


%used to concatinate two sample sets correctly
nbrfiles1 = nposfiles1 + nnegfiles1; 
nbrfiles2 = nposfiles2 + nnegfiles2;
samples = cat(3,samples1(:,:,1:nposfiles1), samples2(:,:,1:nposfiles2), samples1(:,:, nposfiles1+1:nbrfiles1), samples2(:,:,nposfiles2+1:nbrfiles2));
nposfiles = nposfiles1+nposfiles2;
nnegfiles = nnegfiles1+nnegfiles2;
nbrfiles = nposfiles + nnegfiles;

% update nbr of samples if freq has been converted
nbrOfSamples = target_freq*sampling_time;


%% compute core values

[meanFeatures, maxFeatures, minFeatures, kurtosis_samples, skewness_samples, sumFeatures, ...
    meanTiltFeatures, stdFeatures, stdTiltFeatures, sumAllDimFeatures, maxTiltFeatures, ...
    minTiltFeatures, der_mean, der_max, der_min, der_sum, moments, sum_changes, mean_changes,sumAbsFeatures, index_of_first_max, index_of_first_min,max_changes, max_changes_index, tiltXY, tiltXZ, tiltYZ] = extract_base_features( samples,nbrOfSamples,nbrfiles );


%% compute dft values
[ psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_nbrPeaks_tilt, ...
    psdx_nbrPeaks,psdx_mean, psdx_max, psdx_min,psdx_sum, psdx_tilt_mean, psdx_tilt_max, psdx_tilt_min,psdx_tilt_sum, ...
   skewness_psdx, skewness_tilt_psdx, kurtosis_psdx ,kurtosis_tilt_psdx, dft_tilt_k_max_freq, ...
   dft_tilt_k_max_val,dft_samples_k_max_freq, dft_samples_k_max_val, psdx] = extract_dft_features( samples, nposfiles,nnegfiles,nbrOfSamples, target_freq , nbrfiles);


%% Extract correlation features

lag = 30;
[cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, auto_corr, ...
    skewness_acor_samples, kurtosis_acor_samples, sum_changes_auto, mean_changes_auto, der_mean_auto_corr, ...
   der_max_auto_corr ,der_min_auto_corr, der_sum_auto_corr,max_changes_auto, max_changes_index_auto] = extract_corr_features(samples,nposfiles,nnegfiles,lag, nbrfiles);


%% perform training and testing
clc, close all

write_svm_model_to_file = 0;
plot_score_histogram = 0;
attempts = 1000;
alpha = 0.90;
if exist('averageTestRatio')
    averageTestRatioOld = averageTestRatio;
    averageTrainRatioOld = averageTrainRatio;
end



[averageTestRatio, averageTrainRatio, true_positive, false_positive, countMissclassifications,SVMModel, featureVector, ...
    scores_positive_train, scores_negative_train, scores_positive_test, scores_negative_test, mean_train, std_train, SVMModels] ...
= train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max,auto_corr_flat ...
,auto_bins,meanFeatures, meanTiltFeatures, stdFeatures, stdTiltFeatures,sumFeatures,minFeatures,maxFeatures, ...
maxTiltFeatures,minTiltFeatures, skewness_samples, kurtosis_samples, sum_changes, mean_changes, ...
der_min, der_max, der_mean, der_sum ,sumAbsFeatures,sumAllDimFeatures, moments, skewness_acor_samples, kurtosis_acor_samples, ...
sum_changes_auto, mean_changes_auto, der_min_auto_corr, der_max_auto_corr, der_mean_auto_corr, der_sum_auto_corr,write_svm_model_to_file, ...
psdx_nbrPeaks, psdx_nbrPeaks_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, ...
skewness_psdx, skewness_tilt_psdx ,kurtosis_psdx, kurtosis_tilt_psdx, index_of_first_max, index_of_first_min, max_changes_index, max_changes_auto, max_changes_index_auto);


if plot_score_histogram

plot_scores_histogram(scores_positive_train, scores_negative_train, scores_positive_test, scores_negative_test);

end

prescision = true_positive/(true_positive+false_positive);
recall = true_positive;
F1_score = 2*(prescision*recall)/(prescision+recall);

%% plot effect of pivot change for test ratio
[ max_neg, new_ratio_for_pivot_change_neg,min_pos, new_ratio_for_pivot_change_pos ] = no_door_left_behind(scores_positive_test ,scores_negative_test  );
combined =  cat(1, new_ratio_for_pivot_change_neg, new_ratio_for_pivot_change_pos);
combined = combined(2:size(combined,1),:);
combined = sortrows(combined, 1);
plot(combined(:,1),combined(:,2),'r');
hold on
plot(combined(:,1),combined(:,3),'b');

legend('True positive', 'True negative');
title('Accuracy after moving pivot');
%% test svmmodel with test set
[ false_positive_test, true_positive_test, res] = test_model( SVMModel, nbrOfSamples,target_freq, mean_train, std_train );

%% test svmmodel with 1000 svmmodels for all doors
start_door = 1;
end_door = 14; %Door number 14 is all negatives
[ false_positives_test, true_positives_test, res] = test_all_doors( SVMModels, nbrOfSamples ,target_freq,mean_train, std_train, attempts, start_door, end_door);

%% plot feature clustering

[sortedBeta,sortingIndices] = sort(abs(SVMModel.Beta),'descend');
%plot_feature_clustering_1Dx3(psdx_max(1,:),psdx_max(2,:),psdx_max(3,:), nposfiles,nnegfiles);
%plot_feature_clustering_1Dx3(featureVector(:,1)', featureVector(:,8)',featureVector(:,5)', nposfiles,nnegfiles);
%plot_feature_clustering_2D(featureVector(:,5)', featureVector(:,8)',nposfiles,nnegfiles);
plot_feature_clustering_3D(featureVector(:,8)', featureVector(:,5)',featureVector(:,1)',nposfiles,nnegfiles);

%% plot train and testing ratio over alpha
max_alpha = 1.00;
min_alpha = 0.05;
write_svm_model_to_file = 0;
[training_precision, testing_precision] = plot_ratio_over_alpha(min_alpha, max_alpha, attempts, cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, nposfiles, nnegfiles, write_svm_model_to_file);
%% Plot the accuracy as you add features

[added_features_result] = test_k_weighted_features( featureVector,SVMModel.Beta,attempts/2,nposfiles,nnegfiles);
plot(1:41, added_features_result(:,2), 'r', 'Linewidth', 1.5);
hold on;
plot(1:41, 1-added_features_result(:,1), 'b', 'Linewidth', 1.5);
xlabel('Feature #', 'FontSize', 14);
ylabel('Average accuracy',  'FontSize', 14);
set(gca,'fontsize',15)
legend('True positive', 'False positive');
title('False positive and True positive accuracy as features are added');
%% Plot decrease
max_samples = 256;
min_samples = 32;
nbr_steps = 56;
lag = 30;
alpha = 0.9;

write_svm_model_to_file = 0;
[true_positives_decrease_samples,false_positives_decrease_samples] =  plot_decrease_sample_size(samples, max_samples,min_samples, nbr_steps,nposfiles,nnegfiles,lag, 500,alpha, write_svm_model_to_file,target_freq);
%% plot frequency decrease
nbr_steps = 4;
alpha = 0.9;
write_svm_model_to_file = 0;
attempts = 100;

[true_positives, false_positives] = plot_decrease_freq(samples, nbr_steps,nposfiles,nnegfiles, attempts,alpha, write_svm_model_to_file,400)
%% Plot raw data
close all;
x = (1:256)/200;
x = 1:129;
samplestemp = samples/16384;
figure;
plot(x,psdx(:,3,983), 'b', 'Linewidth', 1.5);
hold on
plot(x,psdx(:,3,45), 'r', 'Linewidth', 1.5);
xlabel('Time(s)',  'FontSize', 14);
ylabel('Acceleration (g)',  'FontSize', 10);
set(gca,'fontsize',15)
legend('Negative observation', 'Positive observation');

%% Plot psdx

freq = 0:200/256:100;
plot(freq, psdx(:,3,983), 'b', 'Linewidth', 1.5);
hold on;
plot(freq, psdx(:,3,45), 'r', 'Linewidth', 1.5);
xlabel('Frequency (Hz)', 'FontSize', 14);
ylabel('Power/frequency (db/Hz)',  'FontSize', 14);
set(gca,'fontsize',15)
legend('Negative observation', 'Positive observation');
