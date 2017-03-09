function [true_positives, false_positives] = plot_decrease_sample_size(samples, max_samples,min_samples, nbr_steps,nposfiles,nnegfiles,lag, attempts,alpha)

step_length = (max_samples-min_samples)/nbr_steps;
current_amount_of_samples = max_samples;
true_positives = zeros(1,nbr_steps+1);
false_positives = zeros(1,nbr_steps+1);
for i=1:nbr_steps+1
    
    [cross_corr_max, sum_auto, min_auto, ~, ~, ~, ~] = extract_corr_features(samples(1:current_amount_of_samples,:,:),nposfiles,nnegfiles,lag);
    [~, true_positive, false_positive, ~] = train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max);
    true_positives(i) = true_positive;
    false_positives(i) = false_positive;
    current_amount_of_samples = current_amount_of_samples - step_length
end

plot(linspace(max_samples,min_samples,nbr_steps+1),true_positives,'g');
hold on;
plot(linspace(max_samples,min_samples,nbr_steps+1),1-false_positives,'b');
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here


end

