%% read samples
clear, clc, close all
nbrOfSamples = 256;
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



%% compute correlation 



cross_corr = zeros(3,nposfiles+nnegfiles);

for i = 1 : nposfiles+nnegfiles
    [cross_corr(1,i), ~] = max(abs(xcorr(samples(:,1,i),samples(:,2,i))));
    [cross_corr(2,i), ~] = max(abs(xcorr(samples(:,1,i),samples(:,3,i))));
    [cross_corr(3,i), ~] = max(abs(xcorr(samples(:,2,i),samples(:,3,i))));
    cross_corr(:,i) = cross_corr(:,i) / norm(cross_corr(:,i),2);
end
%20 lags default for auto corr
lag = 30;
auto_corr = zeros(lag+1,3,nposfiles+nnegfiles);
pauto_corr = zeros(lag+1,3,nposfiles+nnegfiles);
auto_corr_pos = zeros(lag+1,nposfiles+nnegfiles);
auto_corr_neg = zeros(lag+1,nposfiles+nnegfiles);

for i = 1 : nposfiles+nnegfiles
    auto_corr(:,1,i) = autocorr(samples(:,1,i),lag);
    auto_corr(:,2,i) = autocorr(samples(:,2,i),lag);
    auto_corr(:,3,i) = autocorr(samples(:,3,i),lag);
    pauto_corr(:,1,i) = parcorr(samples(:,1,i),lag);
    pauto_corr(:,2,i) = parcorr(samples(:,2,i),lag);
    pauto_corr(:,3,i) = parcorr(samples(:,3,i),lag);
    auto_corr_neg(1,i) = sum(auto_corr(:,3,i)<0);
    auto_corr_pos(1,i) = size(auto_corr,1)-auto_corr_neg(1,i);
end

auto_corr_flat = auto_corr_pos - auto_corr_neg;

sum_auto = zeros(nposfiles+nnegfiles,3);
min_auto = zeros(nposfiles+nnegfiles,3);
sum_pauto = zeros(nposfiles+nnegfiles,3);
min_pauto = zeros(nposfiles+nnegfiles,3);
for i = 1 : nposfiles+nnegfiles
    sum_auto(i,:) = sum(squeeze(auto_corr(:,:,i)));
    min_auto(i,:) = min(squeeze(auto_corr(:,:,i)));
    sum_pauto(i,:) = sum(squeeze(pauto_corr(:,:,i)));
    min_pauto(i,:) = min(squeeze(pauto_corr(:,:,i)));
end

% attempt to use bins to utilize for which lag
auto_bins = zeros(nposfiles+nnegfiles,3, floor(lag /6));
%auto_bins_temp = zeros(nposfiles+nnegfiles,3 * floor(lag /6));
for i = 1 : nposfiles+nnegfiles
    for j = 1 : lag/6
         auto_bins(i,:,j) = sum(squeeze(auto_corr((j-1)*floor(lag/6)+1:j*floor(lag/6),:,i)))';
    end
    %auto_bins_temp(i,:) = auto_bins(); 
end



%% perform training and testing
clc
attempts = 1000;
sumOfRatios = 0;
countMissclassifications = zeros(nbrfiles,1);
for i=1:attempts 
alpha = 0.75;
label = zeros(nposfiles+nnegfiles,1);
label(1:nposfiles,1) = 1;
trainIndex = randperm(nposfiles+nnegfiles,floor(alpha*nposfiles+nnegfiles)); 

index= linspace(1,nposfiles+nnegfiles,nposfiles+nnegfiles);
testIndex = setdiff(index,trainIndex);

% combine features
%trainObservations = [squeeze(auto_corr(:,3,trainIndex))' sum_auto(trainIndex,1) min_auto(trainIndex,1)];
trainObservations =  [sum_auto(trainIndex,:) min_auto(trainIndex,:)  cross_corr(:,trainIndex)'];
trainLabels = label(trainIndex);
%testobservations = [squeeze(auto_corr(:,3,testIndex))' sum_auto(testIndex,1) min_auto(testIndex,1)];
testobservations = [sum_auto(testIndex,:) min_auto(testIndex,:) cross_corr(:,testIndex)' ];
testLabels = label(testIndex);
SVMModel = fitclinear(trainObservations,trainLabels);

[pred_labels,score] = predict(SVMModel,testobservations);

%pred_labels 
%testIndex
%score
predictions = (pred_labels == testLabels);
result = zeros(size(predictions,1),2);
result(:,1) = predictions;
result(:,2) = testIndex';
result;


% res = pred_labels - testLabels;
% disp('false positve');
% length(res(res(:)==1))
% disp('false negative');
% length(res(res(:)==-1))
%testLabels-pred_labels
ratio = sum(predictions)/size(predictions,1);
sumOfRatios = sumOfRatios + ratio;
failingIndexes = testIndex(result(:,1) == 0);

countMissclassifications(failingIndexes,1) = countMissclassifications(failingIndexes,1) + 1;

end

averageRatio = sumOfRatios/attempts



