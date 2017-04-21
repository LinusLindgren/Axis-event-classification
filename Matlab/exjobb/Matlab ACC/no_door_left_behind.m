function [ max_neg, new_ratio_neg, min_pos, new_ratio_pos ] = no_door_left_behind(scores_pos_test ,scores_neg_test  )

max_neg = max(scores_neg_test);
i = 0;
new_ratio_neg = [];
while i < max_neg
    pos_ratio = size(scores_pos_test(scores_pos_test >= i),1) / size(scores_pos_test,1);
    neg_ratio = size(scores_neg_test(scores_neg_test < i),1) / size(scores_neg_test,1);
    new_ratio_neg = [new_ratio_neg ; [i pos_ratio neg_ratio]];
    i= i+0.1;
end
new_ratio_pos = [];
i = 0;
min_pos = min(scores_pos_test);
while i > min_pos
    pos_ratio = size(scores_pos_test(scores_pos_test >= i),1) / size(scores_pos_test,1);
    neg_ratio = size(scores_neg_test(scores_neg_test < i),1) / size(scores_neg_test,1);
    new_ratio_pos = [new_ratio_pos ; [i pos_ratio neg_ratio]];
    i= i-0.1;
end
end

