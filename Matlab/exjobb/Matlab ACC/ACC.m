%% read samples
clear, clc, close all
[nposfiles,nnegfiles,samples] = parse_acc_files();

%% compute non-dft values
tiltXZ = calc_tilt(samples(:,3,:),samples(:,1,:));
tiltYZ = calc_tilt(samples(:,3,:),samples(:,2,:));
tiltXY = calc_tilt(samples(:,2,:),samples(:,1,:));

plot_samples(samples,nposfiles,nnegfiles);

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
dft_samples = calc_dft(samples,nposfiles,nnegfiles);
%fix tiltVector
tiltAsSamp = zeros(256,3,nposfiles+nnegfiles);
tiltAsSamp(:,1,:) = tiltXZ;
tiltAsSamp(:,2,:) = tiltYZ;
tiltAsSamp(:,3,:) = tiltXY;
dft_tilt = calc_dft(tiltAsSamp,nposfiles,nnegfiles);

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



