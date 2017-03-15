function [ samples_out, nbr_of_samples] = convert_freq( samples, freq_in, freq_out )


step_length = floor(freq_in / freq_out);
nbr_of_steps = size(samples,1)/ step_length;
samples_out = zeros(nbr_of_steps,size(samples,2),size(samples,3));
for i = 1: nbr_of_steps
    samples_out(i,:,:) = samples(1+(i-1)*step_length,:,:);
end

nbr_of_samples = nbr_of_steps;
    


end

