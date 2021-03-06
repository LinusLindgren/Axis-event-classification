function [true_positives, false_positives] = plot_decrease_sample_size(samples_full, max_samples,min_samples, nbr_steps,nposfiles,nnegfiles,lag, attempts,alpha, write_svm_model_to_file,target_freq)
%plot true and false positive as a function on sampling time
nbrfiles = nposfiles + nnegfiles;
step_length = (max_samples-min_samples)/nbr_steps;
true_positives = zeros(nbr_steps+1,1);
false_positives = zeros(nbr_steps+1,1);
current_amount_of_samples = max_samples;
    
for i=1:nbr_steps+1
    samples = samples_full(1:current_amount_of_samples,:,:);
    
    %extract features
    [meanFeatures, maxFeatures, minFeatures, kurtosis_samples, skewness_samples, sumFeatures, ...
    meanTiltFeatures, stdFeatures, stdTiltFeatures, sumAllDimFeatures, maxTiltFeatures, ...
    minTiltFeatures, der_mean, der_max, der_min, der_sum, moments, sum_changes, mean_changes,sumAbsFeatures, index_of_first_max, index_of_first_min,max_changes, max_changes_index] = extract_base_features( samples,current_amount_of_samples,nbrfiles );

[ psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_nbrPeaks_tilt, ...
    psdx_nbrPeaks,psdx_mean, psdx_max, psdx_min,psdx_sum, psdx_tilt_mean, psdx_tilt_max, psdx_tilt_min,psdx_tilt_sum, ...
   skewness_psdx, skewness_tilt_psdx, kurtosis_psdx ,kurtosis_tilt_psdx, dft_tilt_k_max_freq, ...
   dft_tilt_k_max_val,dft_samples_k_max_freq, dft_samples_k_max_val] = extract_dft_features( samples, nposfiles,nnegfiles,current_amount_of_samples, target_freq , nbrfiles);

lag = 30;
[cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, auto_corr, ...
    skewness_acor_samples, kurtosis_acor_samples, sum_changes_auto, mean_changes_auto, der_mean_auto_corr, ...
   der_max_auto_corr ,der_min_auto_corr, der_sum_auto_corr,max_changes_auto, max_changes_index_auto] = extract_corr_features(samples,nposfiles,nnegfiles,lag, nbrfiles);
   
%train and test
[averageTestRatio, averageTrainRatio, true_positive, false_positive, countMissclassifications,SVMModel, featureVector, ...
    scores_positive_train, scores_negative_train, scores_positive_test, scores_negative_test, mean_train, std_train] ...
= train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max,auto_corr_flat ...
,auto_bins,meanFeatures, meanTiltFeatures, stdFeatures, stdTiltFeatures,sumFeatures,minFeatures,maxFeatures, ...
maxTiltFeatures,minTiltFeatures, skewness_samples, kurtosis_samples, sum_changes, mean_changes, ...
der_min, der_max, der_mean, der_sum ,sumAbsFeatures,sumAllDimFeatures, moments, skewness_acor_samples, kurtosis_acor_samples, ...
sum_changes_auto, mean_changes_auto, der_min_auto_corr, der_max_auto_corr, der_mean_auto_corr, der_sum_auto_corr,write_svm_model_to_file, ...
psdx_nbrPeaks, psdx_nbrPeaks_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, ...
skewness_psdx, skewness_tilt_psdx ,kurtosis_psdx, kurtosis_tilt_psdx, index_of_first_max, index_of_first_min, max_changes_index, max_changes_auto, max_changes_index_auto);
    
%save result
    true_positives(i,1) = true_positive;
    false_positives(i,1) = false_positive;
    current_amount_of_samples = current_amount_of_samples - step_length



end

    %plot result
plot(linspace(max_samples,min_samples,nbr_steps+1)/target_freq,true_positives,'r--o');
hold on;
plot(linspace(max_samples,min_samples,nbr_steps+1)/target_freq,1-false_positives,'b--o');
limits = [0 1.5 0.95 1];
axis(limits);

h = plot(fit_true_positive,'r',x',true_positives','.r');
legend('True positive datapoints','True positive fitted curve','True negative datapoints','True positive fitted curve');



end

