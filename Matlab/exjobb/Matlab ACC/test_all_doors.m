function [ false_positives_test, true_positives_test, res_test] = test_all_doors( SVMModels, nbrOfSamples ,target_freq,mean_train, std_train, attempts, start_door, end_door)
%validate the provided model from data gathered from the doors start_door
%to end_door
sampling_time = 1.28;
false_positives_test = zeros(1,end_door);
true_positives_test = zeros(1,end_door);
%go through all doors
for door=start_door:end_door
door % for feedback
%read the samples from the current door and convert them to the desired
%frequency
[nposfilestest,nnegfilestest,samplestest] = read_samples(400*sampling_time,door,door);
[samplestest, ~] = convert_freq(samplestest,400,target_freq);
nbrfilestest = nposfilestest + nnegfilestest;

%extract features
[meanFeatures, maxFeatures, minFeatures, kurtosis_vec, skewness_vec, sumFeatures, ...
    meanTiltFeatures, stdFeatures, stdTiltFeatures, sumAllDimFeatures, maxTiltFeatures, ...
    minTiltFeatures, der_mean, der_max, der_min, der_sum, moments,sum_changes, mean_changes, ...
    sumAbsFeatures, index_of_first_max, index_of_first_min] = extract_base_features( samplestest,nbrOfSamples,nbrfilestest );

[ psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_nbrPeaks_tilt, ...
    psdx_nbrPeaks,psdx_mean, psdx_max, psdx_min,psdx_sum, psdx_tilt_mean, psdx_tilt_max, psdx_tilt_min,psdx_tilt_sum, ...
   skewness_psdx, skewness_tilt_psdx, kurtosis_psdx ,kurtosis_tilt_psdx, dft_tilt_k_max_freq, ...
   dft_tilt_k_max_val,dft_samples_k_max_freq, dft_samples_k_max_val] = extract_dft_features( samplestest, nposfilestest,nnegfilestest,nbrOfSamples, target_freq , nbrfilestest);

lag = 30;
[cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, auto_corr, ...
    skewness_acor, kurtosis_acor, sum_changes_auto, mean_changes_auto, der_mean_auto_corr, ...
   der_max_auto_corr ,der_min_auto_corr, der_sum_auto_corr] = extract_corr_features(samplestest,nposfilestest,nnegfilestest,lag, nbrfilestest);

%create fearture vector
featureMatrix = [sum_auto(:,3) min_auto(:,2:3) meanFeatures([1],:)' ...
 minFeatures(2:3,:)' ...
maxFeatures(1:2,:)'  skewness_vec(:,2:3) kurtosis_acor(:,[1 3]) skewness_acor ...
sum_changes'  squeeze(psdx_peak_freq_bin_tilt([1],1,:)) squeeze(psdx_peak_freq_bin_tilt(:,3,:))'...
skewness_psdx psdx_nbrPeaks_tilt' ...
squeeze(psdx_peak_freq_bin([1 3 4 5],1,:))' squeeze(psdx_peak_freq_bin([1 5],2,:))' ...
index_of_first_max(3,:)' meanTiltFeatures(2,:)'  stdFeatures(2,:)' ...
sum_changes_auto(1:2,:)' ];

%normalize feature vector
featureMatrix= featureMatrix - repmat(mean_train,size(featureMatrix,1),1);
featureMatrix = featureMatrix ./ repmat(std_train,size(featureMatrix,1),1);
false_positive_sum = 0;
true_positive_sum = 0;
for attempt=1:attempts
    %validate
[pred_labels_test_set,score_test_set] = predict(SVMModels{attempt},featureMatrix);
label = zeros(nposfilestest+nnegfilestest,1);
label(1:nposfilestest,1) = 1;

res_test = pred_labels_test_set - label;
false_positive_test_temp = length(res_test(res_test(:)==1))/(size(label,1)-sum(label));
true_positive_test_temp = 1 - length(res_test(res_test(:)==-1))/sum(label);

false_positive_sum = false_positive_sum + false_positive_test_temp;
true_positive_sum = true_positive_sum + true_positive_test_temp;
end
false_positive_test = false_positive_sum/attempts;
true_positive_test = true_positive_sum/attempts;
false_positives_test(door) = false_positive_test;
true_positives_test(door) = true_positive_test;
end

end
