function [true_positives, false_positives] = plot_decrease_freq(samples_full, nbr_steps,nposfiles,nnegfiles, attempts,alpha, write_svm_model_to_file,max_freq)

nbrfiles = nposfiles + nnegfiles;
true_positives = zeros(nbr_steps+1,1);
false_positives = zeros(nbr_steps+1,1);
current_freq = max_freq;
    %lag = 32;
for i=1:nbr_steps+1
    [samples, current_amount_of_samples] = convert_freq( samples_full, max_freq, current_freq );
    
    
    [meanFeatures, maxFeatures, minFeatures, kurtosis_samples, skewness_samples, sumFeatures, ...
    meanTiltFeatures, stdFeatures, stdTiltFeatures, sumAllDimFeatures, maxTiltFeatures, ...
    minTiltFeatures, der_mean, der_max, der_min, der_sum, moments, sum_changes, mean_changes,sumAbsFeatures, index_of_first_max, index_of_first_min,max_changes, max_changes_index] = extract_base_features( samples,current_amount_of_samples,nbrfiles );

[ psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_nbrPeaks_tilt, ...
    psdx_nbrPeaks,psdx_mean, psdx_max, psdx_min,psdx_sum, psdx_tilt_mean, psdx_tilt_max, psdx_tilt_min,psdx_tilt_sum, ...
   skewness_psdx, skewness_tilt_psdx, kurtosis_psdx ,kurtosis_tilt_psdx, dft_tilt_k_max_freq, ...
   dft_tilt_k_max_val,dft_samples_k_max_freq, dft_samples_k_max_val] = extract_dft_features( samples, nposfiles,nnegfiles,current_amount_of_samples, current_freq , nbrfiles);

if current_freq >= 50
    lag = 30
else
   lag = 10
end
[cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, auto_corr, ...
    skewness_acor_samples, kurtosis_acor_samples, sum_changes_auto, mean_changes_auto, der_mean_auto_corr, ...
   der_max_auto_corr ,der_min_auto_corr, der_sum_auto_corr,max_changes_auto, max_changes_index_auto] = extract_corr_features(samples,nposfiles,nnegfiles,lag, nbrfiles);
   
[averageTestRatio, averageTrainRatio, true_positive, false_positive, countMissclassifications,SVMModel, featureVector, ...
    scores_positive_train, scores_negative_train, scores_positive_test, scores_negative_test, mean_train, std_train] ...
= train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max,auto_corr_flat ...
,auto_bins,meanFeatures, meanTiltFeatures, stdFeatures, stdTiltFeatures,sumFeatures,minFeatures,maxFeatures, ...
maxTiltFeatures,minTiltFeatures, skewness_samples, kurtosis_samples, sum_changes, mean_changes, ...
der_min, der_max, der_mean, der_sum ,sumAbsFeatures,sumAllDimFeatures, moments, skewness_acor_samples, kurtosis_acor_samples, ...
sum_changes_auto, mean_changes_auto, der_min_auto_corr, der_max_auto_corr, der_mean_auto_corr, der_sum_auto_corr,write_svm_model_to_file, ...
psdx_nbrPeaks, psdx_nbrPeaks_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, ...
skewness_psdx, skewness_tilt_psdx ,kurtosis_psdx, kurtosis_tilt_psdx, index_of_first_max, index_of_first_min, max_changes_index, max_changes_auto, max_changes_index_auto);
    
    %lag = lag / 2;
    true_positives(i,1) = true_positive;
    false_positives(i,1) = false_positive;
    current_freq = current_freq / 2
    end

% plot(linspace(max_samples,min_samples,nbr_steps+1),true_positives,'g');
% hold on;
% plot(linspace(max_samples,min_samples,nbr_steps+1),1-false_positives,'b');
xx = [25 50 100 200];
xq= linspace(200,50,36);
true_positives_yy = interp1(xx',true_positives',xq);
false_positives_yy = interp1(xx',false_positives',xq);

fit_true_positives = fit(xq',true_positives_yy','smoothingspline');
fit_false_positives = fit(xq',1-false_positives_yy','smoothingspline');
figure
plot(fit_true_positives,'b',xx',true_positives','.r');
hold on
plot(fit_false_positives,'r',xx',1-false_positives','.b');
legend('Fitted curve true positive ratio','True positive data points', 'Fitted curve true negative ratio', 'True negative data points');
% [X,Y] = meshgrid(linspace(max_samples,min_samples,nbr_steps+1), start_lag:end_lag);
% figure
% mesh(X,Y,true_positives);
% figure
% mesh(X,Y,1-false_positives);
% 
% [max_true_positive,lag_index_true_positive] = max(true_positives);
% corresponding_false_positive = false_positives(lag_index_true_positive,:);
% lag_index_true_positive = lag_index_true_positive + start_lag -1;

%plot(linspace(max_samples,min_samples,nbr_steps+1),true_positives,'g');
%hold on;
%plot(linspace(max_samples,min_samples,nbr_steps+1),1-false_positives,'b');

% [min_false_positive,lag_index_false_positive] = min(false_positives);
% corresponding_true_positive = true_positives(lag_index_false_positive,:);
% lag_index_false_positive = lag_index_false_positive + start_lag -1;
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here


end
