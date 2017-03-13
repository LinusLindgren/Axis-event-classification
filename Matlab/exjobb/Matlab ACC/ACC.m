%% read samples
clear, clc, close all
nbrOfSamples = 160;
[nposfiles,nnegfiles,samples] = parse_acc_files(nbrOfSamples);
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

maxFeatures = squeeze(max(samples,[],1));
minFeatures = squeeze(min(samples,[],1));
maxTiltFeatures = [max(tiltXY,[],1); max(tiltXZ,[],1) ;max(tiltYZ,[],1)];
minTiltFeatures = [min(tiltXY,[],1); min(tiltXZ,[],1) ;min(tiltYZ,[],1)];
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
dft_samples_k_max_freq = zeros(5,nposfiles+nnegfiles);
dft_samples_k_max_val = zeros(5,nposfiles+nnegfiles);


[sortedValuesTiltX,sortIndexTiltX] = sort(dft_tilt(:,1,:),'descend');
[sortedValuesTiltY,sortIndexTiltY] = sort(dft_tilt(:,2,:),'descend');
[sortedValuesTiltZ,sortIndexTiltZ] = sort(dft_tilt(:,3,:),'descend');
dft_tilt_k_max_freq = zeros(5,nposfiles+nnegfiles);
dft_tilt_k_max_val = zeros(5,nposfiles+nnegfiles);

for i = 1 : nposfiles+nnegfiles
        %startIndex = (i-1)*15+1
        for k = 1 : 5
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
[cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat] = extract_corr_features(samples,nposfiles,nnegfiles,lag);

%% perform training and testing
clc
attempts = 1;
alpha = 1.00;

[averageRatio, true_positive, false_positive, countMissclassifications,SVMModel] = train_and_test_scm_model(attempts, alpha, nposfiles,nnegfiles, sum_auto, min_auto, cross_corr_max);

%% Plot decrease
max_samples = 256;
min_samples = 64;
nbr_steps = 8;
[true_positives, false_positives] = plot_decrease_sample_size(samples, max_samples,min_samples, nbr_steps,nposfiles,nnegfiles,lag, attempts,alpha);