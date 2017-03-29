function [cross_corr_max, sum_auto, min_auto, sum_pauto, min_pauto, auto_bins, auto_corr_flat, auto_corr, ...
    skewness_acor_samples, kurtosis_acor_samples, sum_changes_auto, mean_changes_auto, der_mean_auto_corr, ...
   der_max_auto_corr ,der_min_auto_corr, der_sum_auto_corr] = extract_corr_features(samples,nposfiles,nnegfiles,lag, nbrfiles)
%EXTRACT_CORR_FEATURES Summary of this function goes here
%   Detailed explanation goes here

cross_corr_max = zeros(3,nposfiles+nnegfiles);
% cross_corr_sum = zeros(3,nposfiles+nnegfiles);


for i = 1 : nposfiles+nnegfiles
    cross_corr_xy = xcorr(samples(:,1,i),samples(:,2,i));
    cross_corr_xz = xcorr(samples(:,1,i),samples(:,3,i));
    cross_corr_yz = xcorr(samples(:,2,i),samples(:,3,i));
    [cross_corr_max(1,i), ~] = max(abs(cross_corr_xy));
    [cross_corr_max(2,i), ~] = max(abs(cross_corr_xz));
    [cross_corr_max(3,i), ~] = max(abs(cross_corr_yz));
   % cross_corr_max(:,i) = cross_corr_max(:,i) / norm(cross_corr_max(:,i),2);
%     [cross_corr_sum(1,i)] = sum(cross_corr_xy);
%     [cross_corr_sum(2,i)] = sum(cross_corr_xz);
%     [cross_corr_sum(3,i)] = sum(cross_corr_yz);
%     cross_corr_sum(:,i) = cross_corr_sum(:,i) / norm(cross_corr_sum(:,i),2);
end
%20 lags default for auto corr
auto_corr = zeros(lag+1,3,nposfiles+nnegfiles);
pauto_corr = zeros(lag+1,3,nposfiles+nnegfiles);
auto_corr_pos = zeros(1,nposfiles+nnegfiles);
auto_corr_neg = zeros(1,nposfiles+nnegfiles);

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

%auto_corr_flat = auto_corr_pos - auto_corr_neg;
auto_corr_flat = [auto_corr_pos ;auto_corr_neg];

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


end

