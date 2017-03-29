function [ false_positive_test, true_positive_test, res_test] = test_model( SVMModel, nbrOfSamples ,target_freq,mean_train, std_train)
[nposfilestest,nnegfilestest,samplestest] = parse_acc_files(nbrOfSamples * 2,'acc_data\freq400temp\postest\acc*' ...
, 'acc_data\freq400temp\negtest\acc*');
[samplestest, ~] = convert_freq(samplestest,400,target_freq);
nbrfilestest = nposfilestest + nnegfilestest;

[meanFeatures, maxFeatures, minFeatures, kurtosis_samples, skewness_samples, sumFeatures, ...
    meanTiltFeatures, stdFeatures, stdTiltFeatures, sumAllDimFeatures, maxTiltFeatures, ...
    minTiltFeatures, der_mean, der_max, der_min, der_sum, moments,sum_changes, mean_changes, ...
    sumAbsFeatures] = extract_base_features( samplestest,nbrOfSamples,nbrfilestest );

[ psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_nbrPeaks_tilt, ...
    psdx_nbrPeaks,psdx_mean, psdx_max, psdx_min,psdx_sum, psdx_tilt_mean, psdx_tilt_max, psdx_tilt_min,psdx_tilt_sum, ...
   skewness_psdx, skewness_tilt_psdx, kurtosis_psdx ,kurtosis_tilt_psdx, dft_tilt_k_max_freq, ...
   dft_tilt_k_max_val,dft_samples_k_max_freq, dft_samples_k_max_val] = extract_dft_features( samplestest, nposfilestest,nnegfilestest,nbrOfSamples, target_freq , nbrfilestest);

lag = 30;
[cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, auto_corr, ...
    skewness_acor_samples, kurtosis_acor_samples, sum_changes_auto, mean_changes_auto, der_mean_auto_corr, ...
   der_max_auto_corr ,der_min_auto_corr, der_sum_auto_corr] = extract_corr_features(samplestest,nposfilestest,nnegfilestest,lag, nbrfilestest);

featureMatrix = [sum_auto min_auto cross_corr_max(2,:)' meanFeatures' ...
 minFeatures' ...
maxFeatures'  skewness_samples kurtosis_acor_samples skewness_acor_samples ...
sum_changes'  squeeze(psdx_peak_freq_bin_tilt(:,1,:))' squeeze(psdx_peak_freq_bin_tilt(:,2,:))' squeeze(psdx_peak_freq_bin_tilt(:,3,:))'...
skewness_psdx psdx_nbrPeaks_tilt' ...
squeeze(psdx_peak_freq_bin(:,1,:))' squeeze(psdx_peak_freq_bin(:,2,:))'];

featureMatrix= featureMatrix - repmat(mean_train,size(featureMatrix,1),1);
featureMatrix = featureMatrix ./ repmat(std_train,size(featureMatrix,1),1);

[pred_labels_test_set,score_test_set] = predict(SVMModel,featureMatrix);
label = zeros(nposfilestest+nnegfilestest,1);
label(1:nposfilestest,1) = 1;

res_test = pred_labels_test_set - label;
false_positive_test = length(res_test(res_test(:)==1))/(size(label,1)-sum(label));
true_positive_test = 1 - length(res_test(res_test(:)==-1))/sum(label);
end

