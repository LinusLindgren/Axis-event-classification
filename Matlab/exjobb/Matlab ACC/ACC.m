%% read samples
clear, clc, close all
%
nbrOfSamples = 256;

target_freq = 200;
[nposfiles1,nnegfiles1,samples1] = parse_acc_files(nbrOfSamples,'acc_data\freq200temp\postemp\acc*' ...
, 'acc_data\freq200temp\negtemp\acc*');
[samples1, ~] = convert_freq(samples1,200,target_freq);

%nposfiles1=0;
%nnegfiles1=0;

[nposfiles2,nnegfiles2,samples2] = parse_acc_files(nbrOfSamples * 2,'acc_data\freq400temp\postempAll\acc*' ...
, 'acc_data\freq400temp\negtemp1-9\acc*');
[samples2, ~] = convert_freq(samples2,400,target_freq);


%used to concatinate two sample sets correctly
nbrfiles1 = nposfiles1 + nnegfiles1; 
nbrfiles2 = nposfiles2 + nnegfiles2;
samples = cat(3,samples1(:,:,1:nposfiles1), samples2(:,:,1:nposfiles2), samples1(:,:, nposfiles1+1:nbrfiles1), samples2(:,:,nposfiles2+1:nbrfiles2));
%samples = samples2;
nposfiles = nposfiles1+nposfiles2;
nnegfiles = nnegfiles1+nnegfiles2;
nbrfiles = nposfiles + nnegfiles;

% update nbr of samples if freq has been converted
nbrOfSamples = nbrOfSamples *  target_freq / 200;



%% compute non-dft values

[meanFeatures, maxFeatures, minFeatures, kurtosis_samples, skewness_samples, sumFeatures, ...
    meanTiltFeatures, stdFeatures, stdTiltFeatures, sumAllDimFeatures, maxTiltFeatures, ...
    minTiltFeatures, der_mean, der_max, der_min, der_sum, moments, sum_changes, mean_changes,sumAbsFeatures,max_changes, max_changes_index] = extract_base_features( samples,nbrOfSamples,nbrfiles );


%% compute dft values
[ psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_nbrPeaks_tilt, ...
    psdx_nbrPeaks,psdx_mean, psdx_max, psdx_min,psdx_sum, psdx_tilt_mean, psdx_tilt_max, psdx_tilt_min,psdx_tilt_sum, ...
   skewness_psdx, skewness_tilt_psdx, kurtosis_psdx ,kurtosis_tilt_psdx, dft_tilt_k_max_freq, ...
   dft_tilt_k_max_val,dft_samples_k_max_freq, dft_samples_k_max_val] = extract_dft_features( samples, nposfiles,nnegfiles,nbrOfSamples, target_freq , nbrfiles);


%% Extract correlation features

lag = 30;
[cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, auto_corr, ...
    skewness_acor_samples, kurtosis_acor_samples, sum_changes_auto, mean_changes_auto, der_mean_auto_corr, ...
   der_max_auto_corr ,der_min_auto_corr, der_sum_auto_corr,max_changes_auto, max_changes_index_auto] = extract_corr_features(samples,nposfiles,nnegfiles,lag, nbrfiles);




%% perform training and testing
clc, close all
write_svm_model_to_file = 1;
plot_score_histogram = 0;
attempts = 1000;
alpha = 0.9;
%averageTestRatioOld = averageTestRatio;
%averageTrainRatioOld = averageTrainRatio;


[averageTestRatio, averageTrainRatio, true_positive, false_positive, countMissclassifications,SVMModel, featureVector, ...
    scores_positive_train, scores_negative_train, scores_positive_test, scores_negative_test, mean_train, std_train] ...
= train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max,auto_corr_flat ...
,auto_bins,meanFeatures, meanTiltFeatures, stdFeatures, stdTiltFeatures,sumFeatures,minFeatures,maxFeatures, ...
maxTiltFeatures,minTiltFeatures, skewness_samples, kurtosis_samples, sum_changes, mean_changes, ...
der_min, der_max, der_mean, der_sum ,sumAbsFeatures,sumAllDimFeatures, moments, skewness_acor_samples, kurtosis_acor_samples, ...
sum_changes_auto, mean_changes_auto, der_min_auto_corr, der_max_auto_corr, der_mean_auto_corr, der_sum_auto_corr,write_svm_model_to_file, ...
psdx_nbrPeaks, psdx_nbrPeaks_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, ...
skewness_psdx, skewness_tilt_psdx ,kurtosis_psdx, kurtosis_tilt_psdx,max_changes, max_changes_index, max_changes_auto, max_changes_index_auto);


if plot_score_histogram

plot_scores_histogram(scores_positive_train, scores_negative_train, scores_positive_test, scores_negative_test);

end
%% plot effect of pivot change for test ratio
[ max_neg, new_ratio_for_pivot_change ] = no_door_left_behind(scores_positive_test ,scores_negative_test  );
plot(new_ratio_for_pivot_change(:,1),new_ratio_for_pivot_change(:,2),'r');
hold on
plot(new_ratio_for_pivot_change(:,1),new_ratio_for_pivot_change(:,3),'g');

%% test svmmodel with test set
[ false_positive_test, true_positive_test, res] = test_model( SVMModel, nbrOfSamples,target_freq, mean_train, std_train );

%% plot feature clustering

[sortedBeta,sortingIndices] = sort(abs(SVMModel.Beta),'descend');
%plot_feature_clustering_1Dx3(psdx_max(1,:),psdx_max(2,:),psdx_max(3,:), nposfiles,nnegfiles);
%plot_feature_clustering_1Dx3(featureVector(:,sortingIndices(1)),featureVector(:,sortingIndices(2)),featureVector(:,sortingIndices(3)), nposfiles,nnegfiles);
%plot_feature_clustering_2D(featureVector(:,sortingIndices(1))', featureVector(:,sortingIndices(2))',nposfiles,nnegfiles);
plot_feature_clustering_3D(featureVector(:,sortingIndices(1))', featureVector(:,sortingIndices(2))',featureVector(:,sortingIndices(3))',nposfiles,nnegfiles);

%% plot train and testing ratio over alpha
max_alpha = 1.00;
min_alpha = 0.05;
write_svm_model_to_file = 0;
[training_precision, testing_precision] = plot_ratio_over_alpha(min_alpha, max_alpha, attempts, cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, nposfiles, nnegfiles, write_svm_model_to_file);

%% Plot decrease
max_samples = 256;
min_samples = 64;
nbr_steps = 32;
write_svm_model_to_file = 0;
[max_true_positive,min_false_positive ,lag_index_true_positive, lag_index_false_positive , corresponding_false_positive, corresponding_true_positive] = plot_decrease_sample_size(samples, max_samples,min_samples, nbr_steps,nposfiles,nnegfiles,lag, attempts,alpha, write_svm_model_to_file);

