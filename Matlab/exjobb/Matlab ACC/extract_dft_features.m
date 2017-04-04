function [ psdx_peak_power_ratio, psdx_peak_power_ratio_tilt, psdx_peak_freq_bin, psdx_peak_freq_bin_tilt, psdx_nbrPeaks_tilt, ...
    psdx_nbrPeaks,psdx_mean, psdx_max, psdx_min,psdx_sum, psdx_tilt_mean, psdx_tilt_max, psdx_tilt_min,psdx_tilt_sum, ...
   skewness_psdx, skewness_tilt_psdx, kurtosis_psdx ,kurtosis_tilt_psdx, dft_tilt_k_max_freq, ...
   dft_tilt_k_max_val,dft_samples_k_max_freq, dft_samples_k_max_val] = extract_dft_features( samples, nposfiles,nnegfiles,nbrOfSamples, target_freq, nbrfiles )
close all;

tiltXZ = calc_tilt(samples(:,3,:),samples(:,1,:),nbrOfSamples);
tiltYZ = calc_tilt(samples(:,3,:),samples(:,2,:),nbrOfSamples);
tiltXY = calc_tilt(samples(:,2,:),samples(:,1,:),nbrOfSamples);

dft_samples = calc_dft(samples,nposfiles,nnegfiles,nbrOfSamples);
%fix tiltVector
tiltAsSamp = zeros(nbrOfSamples,3,nposfiles+nnegfiles);
tiltAsSamp(:,1,:) = tiltXY;
tiltAsSamp(:,2,:) = tiltXZ;
tiltAsSamp(:,3,:) = tiltYZ;
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

%calc periodogram using fft
N = nbrOfSamples;
%xdft = fft(x);
xdft = dft_samples(1:N/2+1,:,:);
psdx = (1/(target_freq*N)) * abs(xdft(:,:,:)).^2;
psdx(2:end-1,:,:) = 2*psdx(2:end-1,:,:);

xdft_tilt = dft_tilt(1:N/2+1,:,:);
psdx_tilt = (1/(target_freq*N)) * abs(xdft_tilt(:,:,:)).^2;
psdx_tilt(2:end-1,:,:) = 2*psdx_tilt(2:end-1,:,:);

%get nbr peaks within 6 db of highest peak
psdx_nbrPeaks = zeros(3,nbrfiles);
%hårdkodat fixa senare
bin_size = 30 * nbrOfSamples/256;
deci_threshold = 6;
psdx_deci = 10*log10(psdx);
psdx_deci_tilt = 10*log10(psdx_tilt);

psdx_peak_power_ratio = zeros(3,nbrfiles);
psdx_peak_power_ratio_tilt = zeros(3,nbrfiles);
psdx_peak_freq_bin = zeros(ceil(size(psdx_deci,1)/bin_size),3,nbrfiles);
psdx_peak_freq_bin_tilt = zeros(ceil(size(psdx_deci_tilt,1)/bin_size),3,nbrfiles);

psdx_nbrPeaks_tilt = zeros(3,nbrfiles);

for i = 1 : nbrfiles
   max_x = max(psdx_deci(:,1,i));
   max_y = max(psdx_deci(:,2,i));
   max_z = max(psdx_deci(:,3,i));
   max_x_tilt = max(psdx_deci_tilt(:,1,i));
   max_y_tilt = max(psdx_deci_tilt(:,2,i));
   max_z_tilt = max(psdx_deci_tilt(:,3,i));
   for j = 1 : size(psdx_deci,1)
       if psdx_deci(j,1,i) > (max_x-deci_threshold)
          psdx_nbrPeaks(1,i) =  psdx_nbrPeaks(1,i) +1;
          psdx_peak_freq_bin(ceil(j/bin_size),1,i) = psdx_peak_freq_bin(ceil(j/bin_size),1,i) + 1;
          psdx_peak_power_ratio(1,i)= psdx_peak_power_ratio(1,i) + psdx_deci(j,1,i);
       end
       if psdx_deci(j,2,i) > (max_y-deci_threshold)
          psdx_nbrPeaks(2,i) =  psdx_nbrPeaks(2,i) +1;
          psdx_peak_freq_bin(ceil(j/bin_size),2,i) = psdx_peak_freq_bin(ceil(j/bin_size),2,i) + 1;
          psdx_peak_power_ratio(2,i)= psdx_peak_power_ratio(2,i) + psdx_deci(j,2,i);
       end
       if psdx_deci(j,3,i) > (max_z-deci_threshold)
          psdx_nbrPeaks(3,i) =  psdx_nbrPeaks(3,i) +1;
          psdx_peak_freq_bin(ceil(j/bin_size),3,i) = psdx_peak_freq_bin(ceil(j/bin_size),3,i) + 1;
          psdx_peak_power_ratio(3,i)= psdx_peak_power_ratio(3,i) + psdx_deci(j,3,i);
       end
       if psdx_deci_tilt(j,1,i) > (max_x_tilt-deci_threshold)
          psdx_nbrPeaks_tilt(1,i) =  psdx_nbrPeaks_tilt(1,i) +1;
          psdx_peak_freq_bin_tilt(ceil(j/bin_size),1,i) = psdx_peak_freq_bin_tilt(ceil(j/bin_size),1,i) + 1;
          psdx_peak_power_ratio_tilt(1,i)= psdx_peak_power_ratio_tilt(1,i) + psdx_deci_tilt(j,1,i);
       end
       if psdx_deci_tilt(j,2,i) > (max_y_tilt-deci_threshold)
          psdx_nbrPeaks_tilt(2,i) =  psdx_nbrPeaks_tilt(2,i) +1;
          psdx_peak_freq_bin_tilt(ceil(j/bin_size),2,i) = psdx_peak_freq_bin_tilt(ceil(j/bin_size),2,i) + 1;
          psdx_peak_power_ratio_tilt(2,i)= psdx_peak_power_ratio_tilt(2,i) + psdx_deci_tilt(j,2,i);
       end
       if psdx_deci_tilt(j,3,i) > (max_z_tilt-deci_threshold)
          psdx_nbrPeaks_tilt(3,i) =  psdx_nbrPeaks_tilt(3,i) +1;
          psdx_peak_freq_bin_tilt(ceil(j/bin_size),3,i) = psdx_peak_freq_bin_tilt(ceil(j/bin_size),3,i) + 1;
            psdx_peak_power_ratio_tilt(3,i)= psdx_peak_power_ratio_tilt(3,i) + psdx_deci_tilt(j,3,i);
       end
   end
   psdx_peak_power_ratio(:,i) = psdx_peak_power_ratio(:,i) ./ sum(psdx_deci(:,:,i))';
   psdx_peak_power_ratio_tilt(:,i) = psdx_peak_power_ratio_tilt(:,i) ./ sum(psdx_deci_tilt(:,:,i))';
   
end

psdx_mean = squeeze(mean(psdx,1));
psdx_max = squeeze(max(psdx,[],1));
psdx_min = squeeze(min(psdx,[],1));
psdx_sum = squeeze(sum(psdx,1));

skewness_psdx = squeeze(skewness(psdx))';
kurtosis_psdx = squeeze(kurtosis(psdx))';

psdx_tilt_mean = squeeze(mean(psdx_tilt,1));
psdx_tilt_max = squeeze(max(psdx_tilt,[],1));
psdx_tilt_min = squeeze(min(psdx_tilt,[],1));
psdx_tilt_sum = squeeze(sum(psdx_tilt,1));

skewness_tilt_psdx = squeeze(skewness(psdx_tilt))';
kurtosis_tilt_psdx = squeeze(kurtosis(psdx_tilt))';
% freq = 0:target_freq/nbrOfSamples:target_freq/2;
% 
% figure
% plot(freq,10*log10(psdx(:,1,131)))
% 
% figure
% plot(freq,10*log10(psdx(:,1,231)))
% 
% figure
% plot(freq,10*log10(psdx(:,1,631)))
% 
% figure
% plot(freq,10*log10(psdx(:,1,731)))

end

