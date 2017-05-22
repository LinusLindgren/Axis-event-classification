function [dft_samples] = calc_dft( samples,nposfiles,nnegfiles,nbrOfSamples)
 
dft_samples = zeros(nbrOfSamples,3,size(samples,3));
for i = 1 : size(samples,3)
 dft_samples(:,1,i) = fft(samples(:,1,i)); 
 dft_samples(:,2,i) = fft(samples(:,2,i));  
 dft_samples(:,3,i) = fft(samples(:,3,i));  
end

end

