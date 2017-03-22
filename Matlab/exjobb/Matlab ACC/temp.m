%% read samples
clear, clc, close all

res = zeros(6, 5);
l = 2;
for freq=1 : 5 
    k = 1;
    l = l /2;
    for samp_time =1 : 3
%
nbrOfSamples = 256 *k
k = k /2;
target_freq = 200 * l

[nposfiles1,nnegfiles1,samples1] = parse_acc_files(nbrOfSamples,'acc_data\postemp\acc*' ...
, 'acc_data\negtemp\acc*');

[samples1, ~] = convert_freq(samples1,200,target_freq);
%nposfiles1=0;
%nnegfiles1=0;

[nposfiles2,nnegfiles2,samples2] = parse_acc_files(nbrOfSamples * 2,'acc_data\freq400\postemp\acc*' ...
, 'acc_data\freq400\negtemp\acc*');
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


%% Extract correlation features

if(nbrOfSamples > 64)
    lag = 30;
elseif(nbrOfSamples > 20)
    lag = 20;
elseif(nbrOfSamples > 7)
    lag = 7;
else
    lag = 3;
end

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
write_svm_model_to_file = 0;
attempts = 100;
alpha = 0.75;


[ratio, averageTrainRatio, res((samp_time-1)*2+1,freq), res((samp_time-1)*2+2,freq), countMissclassifications,SVMModel] ...
= train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max,auto_corr_flat ...
,auto_bins,meanFeatures, meanTiltFeatures, stdFeatures, stdTiltFeatures,sumFeatures,minFeatures,maxFeatures, ...
maxTiltFeatures,minTiltFeatures, skewness_samples, kurtosis_samples, sum_changes, mean_changes, ...
der_min, der_max, der_mean, der_sum ,sumAbsFeatures,sumAllDimFeatures, moments, skewness_acor_samples, kurtosis_acor_samples, ...
sum_changes_auto, mean_changes_auto, der_min_auto_corr, der_max_auto_corr, der_mean_auto_corr, der_sum_auto_corr,write_svm_model_to_file);
   ratio
    end
end
%% plot 
close all
%128 = 1.28, it is the time of the sampling
ft_128 = fit(x',res(1,:)','smoothingspline');
ff_128 = fit(x',res(2,:)','smoothingspline');
figure
plot(ft_128,'b',x',res(1,:)','.b');
hold on
plot(ff_128,'r',x',res(2,:)','.r');
title('Ratio for 1.28 (s) sampling time for different frequencies');
legend('Data true positive','true positive','data false positive', 'false positive');

ft_64 = fit(x',res(3,:)','smoothingspline');
ff_64 = fit(x',res(4,:)','smoothingspline');
figure
plot(ft_64,'b',x',res(3,:)','.b');
hold on
plot(ff_64,'r',x',res(4,:)','.r');
title('Ratio for 0.64 (s) sampling time for different frequencies');
legend('Data true positive','true positive','data false positive', 'false positive');

ft_32 = fit(x',res(5,:)','smoothingspline');
ff_32 = fit(x',res(6,:)','smoothingspline');
figure
plot(ft_32,'b',x',res(5,:)','.b');
hold on
plot(ff_32,'r',x',res(6,:)','.r');
title('Ratio for 0.32 (s) sampling time for different frequencies');
legend('Data true positive','true positive','data false positive', 'false positive');
