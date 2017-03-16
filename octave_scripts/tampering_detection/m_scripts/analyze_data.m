%
% Script to analyze an event and show all the salient features of that event.
% The event is known (i.e., tilt data from break-in, door-slam, or some such)
%

clear all; close all; clc

% Parameters (to be set)

filename = 'johan/s0.txt';  % name of the file that contains the tilt data
event_type = 'break-in';     % any string will do!
fs = 200; % sampling frequency, Hz.

%
% Filter settings (Moving Avg / Kalman Filter)
%

cfg.maf.len = 10;
cfg.kf.M = 1;
cfg.kf.sigma_x2 = .1;
cfg.kf.sigma_e2 = 35;
mean_tilt = 5; % used in KF
num_peaks = 5; % used in find peaks.


%
% Powers, window lengths etc.
%
cfg.win1.start = 1;
cfg.win1.len = 128;
cfg.win2.start = 129;
cfg.win2.len = 128;

printf("Analyzing file: %s, containing event: %s\n", filename, event_type);

% read the raw tilt data
data.tilt = read_tilt_data_from_file(filename);
printf("Read %d samples.\n", length(data.tilt));

% small sanity check
ll = max(cfg.win1.start+cfg.win1.len-1, cfg.win2.start+cfg.win2.len-1);
if (length(data.tilt) < ll)
  disp('Error! too few data samples!')
  return;
end
printf("\n-----------------------------------------------------");
% filter using a moving average filter
printf("\n Moving Average Filter L = %d\n",cfg.maf.len);
data.maf_out = maf(data.tilt, cfg.maf.len);
printf("\n KF Configuration: ");
printf("\n   process noise variance: %3.4f ", cfg.kf.sigma_x2);
printf("\n   measurement noise variance: %3.4f ", cfg.kf.sigma_e2);
printf("\n");

% filter using a Kalman filter
data.kf_out = kf1d(data.tilt, mean_tilt, cfg.kf);

figure(1); hold on;
plot(data.tilt);
plot(data.maf_out,'r-.');
plot(data.kf_out,'k--');
xlabel('samples');
ylabel('tilt (degrees)');
legend('Raw data', 'MAF output', 'KF output');

% mean tilt over the two windows
data.win1.m = mean(data.tilt(cfg.win1.start:cfg.win1.start+cfg.win1.len-1));
data.win2.m = mean(data.tilt(cfg.win2.start:cfg.win2.start+cfg.win2.len-1));

% power = 1/window_len * sum(x(window).^2)
data.win1.pwr = 1/cfg.win1.len * sum(data.tilt(cfg.win1.start:cfg.win1.start+cfg.win1.len-1).^2);
data.win2.pwr = 1/cfg.win2.len * sum(data.tilt(cfg.win2.start:cfg.win2.start+cfg.win2.len-1).^2);
printf("\n-----------------------------------------------------");
printf("\n Mean tilt for window-1 = %3.4f degrees.", data.win1.m);
printf("\n Mean tilt for window-2 = %3.4f degrees.", data.win2.m);
printf("\n-----------------------------------------------------");
printf("\n Power(window-1) = %3.4f .", data.win1.pwr);
printf("\n Power(window-2) = %3.4f .", data.win2.pwr);
printf("\n-----------------------------------------------------");
% calculate the periodogram (also save the FFT just for debugging purposes) [remove mean]
[data.win1.maf_fft_1, data.win1.maf_s] = periodogram(data.maf_out(cfg.win1.start:cfg.win1.start+cfg.win1.len-1), 1);
[data.win1.maf_fft_2, data.win2.maf_s] = periodogram(data.maf_out(cfg.win2.start:cfg.win2.start+cfg.win2.len-1), 1);
[data.win1.kf_fft_1, data.win1.kf_s] = periodogram(data.kf_out(cfg.win1.start:cfg.win1.start+cfg.win1.len-1), 1);
[data.win1.kf_fft_2, data.win2.kf_s] = periodogram(data.kf_out(cfg.win2.start:cfg.win2.start+cfg.win2.len-1), 1);

figure(2); hold on;
plot(data.win1.maf_s, 'b--*');
plot(data.win1.kf_s, 'k-.o');
title('Window-1 spectrum');
legend('MAF','KF');

figure(3); hold on;
plot(data.win2.maf_s, 'b--*');
plot(data.win2.kf_s, 'k-.o');
title('Window-2 spectrum');
legend('MAF', 'KF');


p_w1_maf = findpeaks(data.win1.maf_s, num_peaks);
p_w2_maf = findpeaks(data.win2.maf_s, num_peaks);
p_w1_kf = findpeaks(data.win1.kf_s, num_peaks);
p_w2_kf = findpeaks(data.win2.kf_s, num_peaks);

printf("\n -- MAF output --");
printf("\n -- Window 1 Peaks --");
for ii = 1:length(p_w1_maf.pamps),
  printf("\n Peak %d : Value %3.4f Position %d Frequency : %3.4f", ii, p_w1_maf.pamps(ii), p_w1_maf.ploc(ii), p_w1_maf.ploc(ii)*fs/cfg.win1.len);
end

printf("\n -- Window 2 Peaks --");
for ii = 1:length(p_w2_maf.pamps),
  printf("\n Peak %d : Value %3.4f Position %d Frequency : %3.4f", ii, p_w2_maf.pamps(ii), p_w2_maf.ploc(ii), p_w2_maf.ploc(ii)*fs/cfg.win2.len);
end
printf("\n-----------------------------------------------------");

printf("\n -- KF output --");
printf("\n -- Window 1 Peaks --");
for ii = 1:length(p_w1_kf.pamps),
  printf("\n Peak %d : Value %3.4f Position %d Frequency : %3.4f", ii, p_w1_kf.pamps(ii), p_w1_kf.ploc(ii), p_w1_kf.ploc(ii)*fs/cfg.win1.len);
end

printf("\n -- Window 2 Peaks --");
for ii = 1:length(p_w2_kf.pamps),
  printf("\n Peak %d : Value %3.4f Position %d Frequency : %3.4f", ii, p_w2_kf.pamps(ii), p_w2_kf.ploc(ii), p_w2_kf.ploc(ii)*fs/cfg.win2.len);
end


printf("\n\n");



