% raw_data_to_sessions (raw_data_file, tilt_threshold, obs_count, pass_count, samples_per_session)
% convert continuously collected accelerometer samples into sessions containing events.
% raw_data_file       : text file containing accelerometer data x y z each row.
% tilt_threshold      : (degrees) tilt detection threshold (example: 3)
% obs_count           : number of tilt counts to observe (example: 5)
% pass_count          : number of counts where tilt exceeds threshold (example: 3)
% samples_per_session : number of samples in each session (for example, 256)
% 
% OUTPUTS: text file (one per session) from s0.txt ... sN.txt
function raw_data_to_session (raw_data_file, tilt_threshold, obs_count, pass_count, samples_per_session)
raw_data = load(raw_data_file);
K = samples_per_session-1;
[N,M] = size(raw_data);
raw_data = raw_data/(2^15-1); % scale properly
printf("There are %d data points with %d axes.\n",N,M);

% calculate tilt. tan^-1 (z/y)
tilt = atan2(raw_data(:,3), raw_data(:,2))*180/pi;

idx = 1;
nidx = 0;
while (idx < N)
  fidx = find(abs(tilt(idx:end)) > 3, 1);
  if (isempty(fidx))
    break;
  end
  fidx=fidx+idx-1;
  % if fewer than obs_count out of max_count tilt values are 
  % smaller than tilt_threshold, dont bother
  if (sum(tilt(fidx:fidx+obs_count) > tilt_threshold) < pass_count)
    idx = fidx+1;
    continue;
  end
  printf("session %d starting at %d\n",nidx,fidx);
  if (fidx+K <= N)
    temp = raw_data(fidx:fidx+K, :);
    filename = ['s',num2str(nidx),'.txt'];
    save("-ascii", filename, "temp");
    tiltfile = ['t',num2str(nidx),'.txt'];
    ts = tilt(fidx:fidx+K);
    save("-ascii",tiltfile,"ts");
    clear temp;
  end
  idx = fidx+K+1;
  nidx = nidx+1;
end

printf("Found %d sessions.\n", nidx);

