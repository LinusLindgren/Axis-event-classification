function [dft_samples] = calc_dft( samples,nposfiles,nnegfiles)
 
dft_samples = zeros(256,3,size(samples,3));
for i = 1 : size(samples,3)
 dft_samples(:,1,i) = fft(samples(:,1,i)); 
 dft_samples(:,2,i) = fft(samples(:,2,i));  
 dft_samples(:,3,i) = fft(samples(:,3,i));  
end



% random_pos_file = floor(rand(1)*nposfiles);
% random_neg_file = floor(rand(1)*nnegfiles)+nposfiles;
% %Plot X,Y,Z Axis for one neg and one pos file seperatly
% figure;
% 
% 
% 
% P2 = abs(dft_samples(:,1,random_pos_file)/256);
% P1 = P2(1:256/2+1);
% P1(2:end-1) = 2*P1(2:end-1);
% f = 200*(0:(256/2))/256;
% plot(f,P1, 'g')
% title('Single-Sided Amplitude Spectrum of X(t)')
% xlabel('f (Hz)')
% ylabel('|P1(f)|')
% hold on;
% P2 = abs(dft_samples(:,1,random_neg_file)/256);
% P1 = P2(1:256/2+1);
% P1(2:end-1) = 2*P1(2:end-1);
% f = 200*(0:(256/2))/256;
% plot(f,P1, 'r')
% title('Single-Sided Amplitude Spectrum of X(t)')
% xlabel('f (Hz)')
% ylabel('|P1(f)|')
% 
% figure;
% P2 = abs(dft_samples(:,2,random_pos_file)/256);
% P1 = P2(1:256/2+1);
% P1(2:end-1) = 2*P1(2:end-1);
% f = 200*(0:(256/2))/256;
% plot(f,P1, 'g')
% title('Single-Sided Amplitude Spectrum of X(t)')
% xlabel('f (Hz)')
% ylabel('|P1(f)|')
% hold on;
% P2 = abs(dft_samples(:,2,random_neg_file)/256);
% P1 = P2(1:256/2+1);
% P1(2:end-1) = 2*P1(2:end-1);
% f = 200*(0:(256/2))/256;
% plot(f,P1, 'r')
% title('Single-Sided Amplitude Spectrum of X(t)')
% xlabel('f (Hz)')
% ylabel('|P1(f)|')
% 
% figure;
% 
% P2 = abs(dft_samples(:,3,random_pos_file)/256);
% P1 = P2(1:256/2+1);
% P1(2:end-1) = 2*P1(2:end-1);
% f = 200*(0:(256/2))/256;
% plot(f,P1, 'g')
% title('Single-Sided Amplitude Spectrum of X(t)')
% xlabel('f (Hz)')
% ylabel('|P1(f)|')
% hold on;
% P2 = abs(dft_samples(:,3,random_neg_file)/256);
% P1 = P2(1:256/2+1);
% P1(2:end-1) = 2*P1(2:end-1);
% f = 200*(0:(256/2))/256;
% plot(f,P1, 'r')
% title('Single-Sided Amplitude Spectrum of X(t)')
% xlabel('f (Hz)')
% ylabel('|P1(f)|')



end

