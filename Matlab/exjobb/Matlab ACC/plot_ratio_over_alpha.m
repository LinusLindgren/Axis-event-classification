function [training_precision, testing_precision] = plot_ratio_over_alpha(min_alpha, max_alpha , attempts, cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, nposfiles, nnegfiles, write_svm_model_to_file)
%PLOT_RATIO_OVER_ALPHA - plot the average train and test ratio as alpha
%which determines size of partioning is varied
alphas = min_alpha:0.05:max_alpha;
count = 1;

training_precision = zeros(size(alphas));
testing_precision = zeros(size(alphas));

for i=min_alpha:0.05:max_alpha
[averageTestRatio, averageTrainRatio, ~, ~, ~,~] = train_and_test_scm_model(attempts, i, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max, write_svm_model_to_file);
training_precision(count) = averageTrainRatio;
testing_precision(count) = averageTestRatio;
count = count + 1;
end

figure;
plot( alphas, training_precision, 'b');
hold on;
plot( alphas, testing_precision, 'g');

end

