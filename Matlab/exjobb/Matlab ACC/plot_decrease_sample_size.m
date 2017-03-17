function [max_true_positive,min_false_positive ,lag_index_true_positive, lag_index_false_positive , corresponding_false_positive, corresponding_true_positive] = plot_decrease_sample_size(samples, max_samples,min_samples, nbr_steps,nposfiles,nnegfiles,lag, attempts,alpha, write_svm_model_to_file)
start_lag = 25;
end_lag = 26;

step_length = (max_samples-min_samples)/nbr_steps;
true_positives = zeros(end_lag-start_lag,nbr_steps+1);
false_positives = zeros(end_lag-start_lag,nbr_steps+1);

for lag = start_lag : end_lag
    current_amount_of_samples = max_samples;
    for i=1:nbr_steps+1
    
    [cross_corr_max, sum_auto, min_auto, ~, ~, ~, ~] = extract_corr_features(samples(1:current_amount_of_samples,:,:),nposfiles,nnegfiles,lag);
    [~, true_positive, false_positive, ~] = train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max, write_svm_model_to_file);
    true_positives(lag-start_lag+1,i) = true_positive;
    false_positives(lag-start_lag+1,i) = false_positive;
    current_amount_of_samples = current_amount_of_samples - step_length
end

%plot(linspace(max_samples,min_samples,nbr_steps+1),true_positives,'g');
%hold on;
%plot(linspace(max_samples,min_samples,nbr_steps+1),1-false_positives,'b');

end

[X,Y] = meshgrid(linspace(max_samples,min_samples,nbr_steps+1), start_lag:end_lag);
figure
mesh(X,Y,true_positives);
figure
mesh(X,Y,1-false_positives);

[max_true_positive,lag_index_true_positive] = max(true_positives);
corresponding_false_positive = false_positives(lag_index_true_positive,:);
lag_index_true_positive = lag_index_true_positive + start_lag -1;

%plot(linspace(max_samples,min_samples,nbr_steps+1),true_positives,'g');
%hold on;
%plot(linspace(max_samples,min_samples,nbr_steps+1),1-false_positives,'b');

[min_false_positive,lag_index_false_positive] = min(false_positives);
corresponding_true_positive = true_positives(lag_index_false_positive,:);
lag_index_false_positive = lag_index_false_positive + start_lag -1;
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here


end

