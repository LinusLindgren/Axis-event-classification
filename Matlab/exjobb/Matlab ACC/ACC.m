%% read samples
clear, clc, close all
nbrOfSamples = 256;

[nposfiles1,nnegfiles1,samples1] = parse_acc_files(nbrOfSamples,'acc_data\postemp\acc*' ...
, 'acc_data\negtemp\acc*');
%[samples1, ~] = convert_freq(samples1,200,100);
%nposfiles1=0;
%nnegfiles1=0;

[nposfiles2,nnegfiles2,samples2] = parse_acc_files(nbrOfSamples * 2,'acc_data\freq400\postemp\acc*' ...
, 'acc_data\freq400\negtemp\acc*');
[samples2, ~] = convert_freq(samples2,400,200);

%used to concatinate two sample sets correctly
nbrfiles1 = nposfiles1 + nnegfiles1; 
nbrfiles2 = nposfiles2 + nnegfiles2;
samples = cat(3,samples1(:,:,1:nposfiles1), samples2(:,:,1:nposfiles2), samples1(:,:, nposfiles1+1:nbrfiles1), samples2(:,:,nposfiles2+1:nbrfiles2));
%samples = samples2;
nposfiles = nposfiles1+nposfiles2;
nnegfiles = nnegfiles1+nnegfiles2;
nbrfiles = nposfiles + nnegfiles;

%% compute non-dft values

tiltXZ = calc_tilt(samples(:,3,:),samples(:,1,:),nbrOfSamples);
tiltYZ = calc_tilt(samples(:,3,:),samples(:,2,:),nbrOfSamples);
tiltXY = calc_tilt(samples(:,2,:),samples(:,1,:),nbrOfSamples);

%plot_samples(samples,nposfiles,nnegfiles,nbrOfSamples);

meanFeatures = squeeze(mean(samples,1));
meanTiltFeatures = [mean(tiltXY,1); mean(tiltXZ,1) ;mean(tiltYZ,1)];


stdFeatures =  squeeze(std(samples,0,1));
stdTiltFeatures =  [std(tiltXY,0,1); std(tiltXZ,0,1) ;std(tiltYZ,0,1)];
sumFeatures = squeeze(sum(samples,1));
sumAbsFeatures = squeeze(sum(abs(samples),1));
sumAllDimFeatures = squeeze(sum(sumAbsFeatures,1));
maxFeatures = squeeze(max(samples,[],1));
minFeatures = squeeze(min(samples,[],1));
maxTiltFeatures = [max(tiltXY,[],1); max(tiltXZ,[],1) ;max(tiltYZ,[],1)];
minTiltFeatures = [min(tiltXY,[],1); min(tiltXZ,[],1) ;min(tiltYZ,[],1)];

skewness_samples = squeeze(skewness(samples))';
kurtosis_samples = squeeze(kurtosis(samples))';

sum_changes = zeros(3,nbrfiles);
mean_changes = zeros(3,nbrfiles);
for i = 1: nbrfiles
   for j = 1 : nbrOfSamples -1
       sum_changes(1,i) = sum_changes(1,i) + abs(samples(j+1,1,i) -samples(j,1,i));
       sum_changes(2,i) = sum_changes(2,i) + abs(samples(j+1,2,i) -samples(j,2,i));
       sum_changes(3,i) = sum_changes(3,i) + abs(samples(j+1,3,i) -samples(j,3,i));
   end
   mean_changes(1,i) = sum_changes(1,i)/(nbrOfSamples-1);
   mean_changes(2,i) = sum_changes(2,i)/(nbrOfSamples-1);
   mean_changes(3,i) = sum_changes(3,i)/(nbrOfSamples-1);
end

derivate = diff(samples);
der_mean = squeeze(mean(derivate,1));
der_max = squeeze(max(derivate,[],1));
der_min = squeeze(min(derivate,[],1));
der_sum = squeeze(sum(derivate,1));
moments = squeeze(moment(samples,3));

%% compute dft values
close all;
dft_samples = calc_dft(samples,nposfiles,nnegfiles,nbrOfSamples);
%fix tiltVector
tiltAsSamp = zeros(nbrOfSamples,3,nposfiles+nnegfiles);
tiltAsSamp(:,1,:) = tiltXZ;
tiltAsSamp(:,2,:) = tiltYZ;
tiltAsSamp(:,3,:) = tiltXY;
dft_tilt = calc_dft(tiltAsSamp,nposfiles,nnegfiles,nbrOfSamples);

[sortedValuesSamplesX,sortIndexSamplesX] = sort(dft_samples(:,1,:),'descend');
[sortedValuesSamplesY,sortIndexSamplesY] = sort(dft_samples(:,2,:),'descend');
[sortedValuesSamplesZ,sortIndexSamplesZ] = sort(dft_samples(:,3,:),'descend');
%dft_samples_k_max_freq = zeros(5*3*(nposfiles+nnegfiles),1);
max_k_freq = 5;
dft_samples_k_max_freq = zeros(max_k_freq,nposfiles+nnegfiles);
dft_samples_k_max_val = zeros(max_k_freq,nposfiles+nnegfiles);


[sortedValuesTiltX,sortIndexTiltX] = sort(dft_tilt(:,1,:),'descend');
[sortedValuesTiltY,sortIndexTiltY] = sort(dft_tilt(:,2,:),'descend');
[sortedValuesTiltZ,sortIndexTiltZ] = sort(dft_tilt(:,3,:),'descend');
dft_tilt_k_max_freq = zeros(max_k_freq,nposfiles+nnegfiles);
dft_tilt_k_max_val = zeros(max_k_freq,nposfiles+nnegfiles);

for i = 1 : nposfiles+nnegfiles
        %startIndex = (i-1)*15+1
        for k = 1 : max_k_freq
    dft_samples_k_max_freq(k,i) = squeeze(sortIndexSamplesX(k,1,i));
    dft_samples_k_max_freq(k,i) = squeeze(sortIndexSamplesY(k,1,i));
    dft_samples_k_max_freq(k,i) = squeeze(sortIndexSamplesZ(k,1,i));
    dft_samples_k_max_val(k,i) = squeeze(sortedValuesSamplesX(k,1,i));
    dft_samples_k_max_val(k,i) = squeeze(sortedValuesSamplesY(k,1,i));
    dft_samples_k_max_val(k,i) = squeeze(sortedValuesSamplesZ(k,1,i));
    dft_tilt_k_max_freq(k,i) = squeeze(sortIndexTiltX(k,1,i));
    dft_tilt_k_max_freq(k,i) = squeeze(sortIndexTiltY(k,1,i));
    dft_tilt_k_max_freq(k,i) = squeeze(sortIndexTiltZ(k,1,i));
    dft_tilt_k_max_val(k,i) = squeeze(sortedValuesTiltX(k,1,i));
    dft_tilt_k_max_val(k,i) = squeeze(sortedValuesTiltY(k,1,i));
    dft_tilt_k_max_val(k,i) = squeeze(sortedValuesTiltZ(k,1,i));
        end
end



%% Extract correlation features

lag = 30;
[cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, auto_corr] = extract_corr_features(samples,nposfiles,nnegfiles,lag);
skewness_acor_samples = squeeze(skewness(auto_corr))';
kurtosis_acor_samples = squeeze(kurtosis(auto_corr))';

sum_changes_auto = zeros(3,nbrfiles);
mean_changes_auto = zeros(3,nbrfiles);
for i = 1: nbrfiles
   for j = 1 : lag
       sum_changes_auto(1,i) = sum_changes_auto(1,i) + abs(auto_corr(j+1,1,i) -auto_corr(j,1,i));
       sum_changes_auto(2,i) = sum_changes_auto(2,i) + abs(auto_corr(j+1,2,i) -auto_corr(j,2,i));
       sum_changes_auto(3,i) = sum_changes_auto(3,i) + abs(auto_corr(j+1,3,i) -auto_corr(j,3,i));
   end
   mean_changes_auto(1,i) = sum_changes_auto(1,i)/(lag);
   mean_changes_auto(2,i) = sum_changes_auto(2,i)/(lag);
   mean_changes_auto(3,i) = sum_changes_auto(3,i)/(lag);
end

derivate_auto_corr = diff(auto_corr);
der_mean_auto_corr = squeeze(mean(derivate_auto_corr,1));
der_max_auto_corr = squeeze(max(derivate_auto_corr,[],1));
der_min_auto_corr = squeeze(min(derivate_auto_corr,[],1));
der_sum_auto_corr = squeeze(sum(derivate_auto_corr,1));


%% perform training and testing
clc
write_svm_model_to_file = 0;
attempts = 1000;
alpha = 0.75;


[averageTestRatio, averageTrainRatio, true_positive, false_positive, countMissclassifications,SVMModel] ...
= train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max,auto_corr_flat ...
,auto_bins,meanFeatures, meanTiltFeatures, stdFeatures, stdTiltFeatures,sumFeatures,minFeatures,maxFeatures, ...
maxTiltFeatures,minTiltFeatures, skewness_samples, kurtosis_samples, sum_changes, mean_changes, ...
der_min, der_max, der_mean, der_sum ,sumAbsFeatures,sumAllDimFeatures, moments, skewness_acor_samples, kurtosis_acor_samples, ...
sum_changes_auto, mean_changes_auto, der_min_auto_corr, der_max_auto_corr, der_mean_auto_corr, der_sum_auto_corr,write_svm_model_to_file);

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